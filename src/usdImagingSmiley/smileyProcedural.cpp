//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "smileyProcedural.h"

#include "tokens.h"

#include "pxr/base/gf/math.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/type.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/imaging/hd/basisCurvesSchema.h"
#include "pxr/imaging/hd/basisCurvesTopologySchema.h"
#include "pxr/imaging/hd/meshSchema.h"
#include "pxr/imaging/hd/meshTopologySchema.h"
#include "pxr/imaging/hd/overlayContainerDataSource.h"
#include "pxr/imaging/hd/primOriginSchema.h"
#include "pxr/imaging/hd/primvarSchema.h"
#include "pxr/imaging/hd/primvarsSchema.h"
#include "pxr/imaging/hd/purposeSchema.h"
#include "pxr/imaging/hd/retainedDataSource.h"
#include "pxr/imaging/hd/sphereSchema.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hd/xformSchema.h"
#include "pxr/imaging/hdGp/generativeProceduralPlugin.h"
#include "pxr/imaging/hdGp/generativeProceduralPluginRegistry.h"

PXR_NAMESPACE_OPEN_SCOPE

// A unit, upward-pointing triangle centered at the origin, described as
// 3 points and a single triangular face.
static const VtVec3fArray&
_GetUnitTrianglePoints()
{
    static const VtVec3fArray points = {
        { 0.0f,  0.0f,  0.5f},
        {-0.5f,  0.0f, -0.5f},
        { 0.5f,  0.0f, -0.5f}
    };
    return points;
}

static const VtIntArray&
_GetUnitTriangleFaceVertexCounts()
{
    static const VtIntArray counts = {3};
    return counts;
}

static const VtIntArray&
_GetUnitTriangleFaceVertexIndices()
{
    static const VtIntArray indices = {0, 1, 2};
    return indices;
}

static HdDataSourceBaseHandle
_GetDisplayColorDataSource(const VtVec3fArray &color)
{
    return HdPrimvarSchema::Builder()
        .SetPrimvarValue(
            HdRetainedTypedSampledDataSource<VtVec3fArray>::New(color))
        .SetInterpolation(
            HdPrimvarSchema::BuildInterpolationDataSource(
                HdPrimvarSchemaTokens->constant))
        .Build();
}

// Builds an eye mesh: a unit triangle scaled by 'eyeSize', with points and
// a displayColor primvar.
static HdContainerDataSourceHandle
_GetEyeMeshDataSource(double eyeSize, const VtVec3fArray &color)
{
    VtVec3fArray points = _GetUnitTrianglePoints();
    for (GfVec3f &pt : points) {
        pt *= static_cast<float>(eyeSize);
    }

    static const TfToken primvarTokens[] = {HdTokens->points, HdTokens->displayColor};
    const HdDataSourceBaseHandle primvarValues[] = {
        HdPrimvarSchema::Builder()
            .SetPrimvarValue(
                HdRetainedTypedSampledDataSource<VtVec3fArray>::New(points))
            .SetInterpolation(
                HdPrimvarSchema::BuildInterpolationDataSource(
                    HdPrimvarSchemaTokens->vertex))
            .SetRole(
                HdPrimvarSchema::BuildRoleDataSource(
                    HdPrimvarSchemaTokens->point))
            .Build(),
        _GetDisplayColorDataSource(color)
    };

    return HdRetainedContainerDataSource::New(
        HdMeshSchema::GetSchemaToken(),
        HdMeshSchema::Builder()
            .SetTopology(
                HdMeshTopologySchema::Builder()
                    .SetFaceVertexCounts(
                        HdRetainedTypedSampledDataSource<VtIntArray>::New(
                            _GetUnitTriangleFaceVertexCounts()))
                    .SetFaceVertexIndices(
                        HdRetainedTypedSampledDataSource<VtIntArray>::New(
                            _GetUnitTriangleFaceVertexIndices()))
                    .SetOrientation(
                        HdMeshTopologySchema::BuildOrientationDataSource(
                            HdMeshTopologySchemaTokens->rightHanded))
                    .Build())
            .Build(),

        HdPurposeSchema::GetSchemaToken(),
        HdPurposeSchema::Builder()
            .SetPurpose(HdRetainedTypedSampledDataSource<TfToken>::New(
                            HdRenderTagTokens->geometry))
            .Build(),

        HdPrimvarsSchema::GetSchemaToken(),
        HdPrimvarsSchema::BuildRetained(2, primvarTokens, primvarValues)
    );
}

// Records 'originPath' (the Smiley prim's own path) as this synthesized
// child's prim origin, so that selecting the Smiley prim in USD also
// selects/highlights the generated eye and mouth prims.
static HdContainerDataSourceHandle
_GetPrimOriginDataSource(const SdfPath &originPath)
{
    return HdRetainedContainerDataSource::New(
        HdPrimOriginSchemaTokens->primOrigin,
        HdRetainedContainerDataSource::New(
            HdPrimOriginSchemaTokens->scenePath,
            HdRetainedTypedSampledDataSource<HdPrimOriginSchema::OriginPath>::New(
                HdPrimOriginSchema::OriginPath(originPath))));
}

// Places an eye mesh at 'center' via an xform overlay. 'proceduralXform' is
// the procedural prim's own (already-inherited) transform: since this child
// is synthesized rather than authored on the stage, it never passes through
// the flattening that composes ancestor transforms, so that composition has
// to happen here. resetXformStack is set so nothing downstream re-composes
// this already-final matrix with the procedural prim's transform again.
static HdContainerDataSourceHandle
_GetPlacedEyeDataSource(
    double eyeSize, const GfVec3d& center, const GfMatrix4d &proceduralXform, const VtVec3fArray& color)
{
    const HdContainerDataSourceHandle meshDs = _GetEyeMeshDataSource(eyeSize, color);

    const GfMatrix4d matrix =
        GfMatrix4d(1.0).SetTranslateOnly(center) * proceduralXform;

    return HdOverlayContainerDataSource::New(
        HdRetainedContainerDataSource::New(
            HdXformSchema::GetSchemaToken(),
            HdXformSchema::Builder()
                .SetMatrix(
                    HdRetainedTypedSampledDataSource<GfMatrix4d>::New(matrix))
                .SetResetXformStack(
                    HdRetainedTypedSampledDataSource<bool>::New(true))
                .Build()),
        meshDs);
}

// Builds the "mouth" basis curve: a single polyline sampled across the
// width of the face. The curve is a parabola whose curvature is driven by
// 'smile': the corners are level with the middle at smile = 0.5, rise
// above the middle (a smile) as smile approaches 1, and drop below the
// middle (a frown) as smile approaches 0.
static HdContainerDataSourceHandle
_GetMouthCurveDataSource(double smile, const GfMatrix4d &proceduralXform, const VtVec3fArray &color)
{
    constexpr int numPoints = 11;
    constexpr float halfWidth = 0.5f;
    constexpr float baseZ = -0.4f;
    constexpr float amplitude = 0.35f;
    const float curveFactor = static_cast<float>(2.0 * smile - 1.0);

    VtVec3fArray points(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        const float t = static_cast<float>(i) / (numPoints - 1); // [0, 1]
        const float x = -halfWidth + t * (2.0f * halfWidth);
        const float u = x / halfWidth; // [-1, 1]
        const float z = baseZ + amplitude * curveFactor * u * u;
        points[i] = GfVec3f(x, 0.0f, z);
    }
    const VtIntArray curveVertexCounts = {numPoints};

    static const TfToken primvarTokens[] = {HdTokens->points, HdTokens->displayColor};
    const HdDataSourceBaseHandle primvarValues[] = {
        HdPrimvarSchema::Builder()
            .SetPrimvarValue(
                HdRetainedTypedSampledDataSource<VtVec3fArray>::New(points))
            .SetInterpolation(
                HdPrimvarSchema::BuildInterpolationDataSource(
                    HdPrimvarSchemaTokens->vertex))
            .SetRole(
                HdPrimvarSchema::BuildRoleDataSource(
                    HdPrimvarSchemaTokens->point))
            .Build(),
        _GetDisplayColorDataSource(color)
    };

    return HdRetainedContainerDataSource::New(
        HdBasisCurvesSchema::GetSchemaToken(),
        HdBasisCurvesSchema::Builder()
            .SetTopology(
                HdBasisCurvesTopologySchema::Builder()
                    .SetCurveVertexCounts(
                        HdRetainedTypedSampledDataSource<VtIntArray>::New(
                            curveVertexCounts))
                    .SetBasis(
                        HdRetainedTypedSampledDataSource<TfToken>::New(
                            HdTokens->bezier))
                    .SetType(
                        HdRetainedTypedSampledDataSource<TfToken>::New(
                            HdTokens->linear))
                    .SetWrap(
                        HdRetainedTypedSampledDataSource<TfToken>::New(
                            HdTokens->nonperiodic))
                    .Build())
            .Build(),

        HdPurposeSchema::GetSchemaToken(),
        HdPurposeSchema::Builder()
            .SetPurpose(HdRetainedTypedSampledDataSource<TfToken>::New(
                            HdRenderTagTokens->geometry))
            .Build(),

        HdPrimvarsSchema::GetSchemaToken(),
        HdPrimvarsSchema::BuildRetained(2, primvarTokens, primvarValues),

        HdXformSchema::GetSchemaToken(),
        HdXformSchema::Builder()
            .SetMatrix(
                HdRetainedTypedSampledDataSource<GfMatrix4d>::New(
                    proceduralXform))
            .SetResetXformStack(
                HdRetainedTypedSampledDataSource<bool>::New(true))
            .Build()
    );
}

// Reads a plain, non-primvar double value (as published by
// UsdImagingSmileyAdapter) off the procedural prim's own data source,
// clamped to [0, 1].
static double
_GetArgValue(
    const HdContainerDataSourceHandle &proceduralPrimDs,
    const TfToken &argName,
    double fallback)
{
    if (proceduralPrimDs) {
        if (HdSampledDataSourceHandle valueDs =
                HdSampledDataSource::Cast(proceduralPrimDs->Get(argName))) {
            const VtValue value = valueDs->GetValue(0.0f);
            if (value.IsHolding<double>()) {
                return GfClamp(value.UncheckedGet<double>(), 0.0, 1.0);
            }
        }
    }
    return fallback;
}

// Reads the resolved 'target' relationship path (as published by
// UsdImagingSmileyAdapter) off the procedural prim's own data source.
// Returns an empty path if no single target is set.
static SdfPath
_GetTargetPath(const HdContainerDataSourceHandle &proceduralPrimDs)
{
    if (proceduralPrimDs) {
        if (HdSampledDataSourceHandle valueDs = HdSampledDataSource::Cast(
                proceduralPrimDs->Get(UsdImagingSmileyTokens->smileyTarget))) {
            const VtValue value = valueDs->GetValue(0.0f);
            if (value.IsHolding<VtArray<SdfPath>>()) {
                const VtArray<SdfPath> &targets =
                    value.UncheckedGet<VtArray<SdfPath>>();
                if (targets.size() == 1) {
                    return targets[0];
                }
            }
        }
    }
    return SdfPath();
}

// Reads the resolved matrix off a prim's HdXformSchema, or the identity if
// absent.
static GfMatrix4d
_GetXform(const HdContainerDataSourceHandle &primDs)
{
    if (HdXformSchema xformSchema = HdXformSchema::GetFromParent(primDs)) {
        if (HdMatrixDataSourceHandle matrixDs = xformSchema.GetMatrix()) {
            return matrixDs->GetTypedValue(0);
        }
    }
    return GfMatrix4d(1.0);
}

// Reads the resolved radius off a prim's HdSphereSchema, or 'fallback' if
// the prim isn't a sphere / has no radius.
static double
_GetSphereRadius(const HdContainerDataSourceHandle &primDs, double fallback)
{
    if (HdSphereSchema sphereSchema = HdSphereSchema::GetFromParent(primDs)) {
        if (HdDoubleDataSourceHandle radiusDs = sphereSchema.GetRadius()) {
            return radiusDs->GetTypedValue(0);
        }
    }
    return fallback;
}

UsdImagingSmileyProcedural::UsdImagingSmileyProcedural(
    const SdfPath& proceduralPrimPath)
    : HdGpGenerativeProcedural(proceduralPrimPath)
{
}

UsdImagingSmileyProcedural::~UsdImagingSmileyProcedural() = default;

HdGpGenerativeProcedural::DependencyMap
UsdImagingSmileyProcedural::UpdateDependencies(
    const HdSceneIndexBaseRefPtr &inputScene)
{
    // eyeSize, smile, and target live on this
    // procedural's own prim, so no dependency declaration is needed for
    // them. But if target is set, the anchor's xform on the target
    // prim is also an input: declare a dependency on it so that moving the
    // target re-cooks this procedural.
    HdGpGenerativeProcedural::DependencyMap map{};

    const HdSceneIndexPrim proceduralPrim =
        inputScene->GetPrim(_GetProceduralPrimPath());
    const SdfPath targetPath = _GetTargetPath(proceduralPrim.dataSource);
    if (!targetPath.IsEmpty()) {
        map[targetPath] = {
            HdXformSchema::GetDefaultLocator(),
            HdSphereSchema::GetDefaultLocator()
        };
    }

    return map;
}

HdGpGenerativeProcedural::ChildPrimTypeMap
UsdImagingSmileyProcedural::Update(
    const HdSceneIndexBaseRefPtr &inputScene,
    const ChildPrimTypeMap &previousResult,
    const DependencyMap &dirtiedDependencies,
    HdSceneIndexObserver::DirtiedPrimEntries *outputDirtiedPrims)
{
    const HdSceneIndexPrim proceduralPrim =
        inputScene->GetPrim(_GetProceduralPrimPath());

    const double eyeSize = _GetArgValue(
        proceduralPrim.dataSource, UsdImagingSmileyTokens->smileyEyeSize, 0.3);
    const double smile = _GetArgValue(
        proceduralPrim.dataSource, UsdImagingSmileyTokens->smileySmile, 1.0);

    // The anchor transform that the generated children need to bake into
    // their own matrices: as synthesized prims, they never pass through the
    // flattening that composes ancestor transforms for prims authored on the
    // stage. If target is set, anchor at that prim's (local) xform
    // instead of the procedural prim's own; eyeSize/smile
    // still control the face shape independently of the anchor.
    const SdfPath targetPath = _GetTargetPath(proceduralPrim.dataSource);
    GfMatrix4d proceduralXform(1.0);
    if (targetPath.IsEmpty()) {
        proceduralXform = _GetXform(proceduralPrim.dataSource);
    } else {
        const HdContainerDataSourceHandle targetDs =
            inputScene->GetPrim(targetPath).dataSource;
        // The eye/mouth mesh is built flat in the local XZ plane (Z is its
        // own "vertical" axis; X is horizontal), so -Y is its outward-facing
        // normal (per the eye triangle's rightHanded winding). Push the face
        // out along -Y to just past the sphere's radius, like a decal
        // resting on its surface, rather than floating in front of it.
        const double radius = _GetSphereRadius(targetDs, 1.0);
        constexpr double kSurfaceMargin = 0.05;
        const GfMatrix4d offset =
            GfMatrix4d(1.0).SetTranslateOnly(
                GfVec3d(0.0, -(radius + kSurfaceMargin), 0.0));
        proceduralXform = offset * _GetXform(targetDs);
    }

    if (HdPrimvarsSchema primvarsSchema = HdPrimvarsSchema::GetFromParent(proceduralPrim.dataSource)) {
        if (HdPrimvarSchema primvarSchema = primvarsSchema.GetPrimvar(HdTokens->displayColor)) {
            const VtValue value = primvarSchema.GetPrimvarValue()->GetValue(0.0f);
            if (value.IsHolding<VtVec3fArray>()) {
                _displayColor = value.UncheckedGet<VtVec3fArray>();
            }
        }
    }

    const SdfPath leftEyePath =
        _GetProceduralPrimPath().AppendChild(UsdImagingSmileyTokens->leftEye);
    const SdfPath rightEyePath =
        _GetProceduralPrimPath().AppendChild(UsdImagingSmileyTokens->rightEye);
    const SdfPath mouthPath =
        _GetProceduralPrimPath().AppendChild(UsdImagingSmileyTokens->mouth);

    bool argsChanged = false;
    {
        std::lock_guard<std::mutex> lock(_argsMutex);
        argsChanged = (eyeSize != _eyeSize || smile != _smile ||
                       proceduralXform != _proceduralXform);
        if (argsChanged) {
            _eyeSize = eyeSize;
            _smile = smile;
            _proceduralXform = proceduralXform;
        }
    }

    if (argsChanged) {
        if (outputDirtiedPrims && !previousResult.empty()) {
            outputDirtiedPrims->emplace_back(
                leftEyePath, HdDataSourceLocatorSet::UniversalSet());
            outputDirtiedPrims->emplace_back(
                rightEyePath, HdDataSourceLocatorSet::UniversalSet());
            outputDirtiedPrims->emplace_back(
                mouthPath, HdDataSourceLocatorSet::UniversalSet());
        }
    }

    // TODO:
        // If async enabled, do not populate child prim type map
            // Just get eyesize, smile, and target
        // Start 2 different threads - one for the eyes, one for the smile
        // Have async update check the status of those threads
        // If update is called again, safely terminate those threads
        // AsyncUpdate can simply check the result of those threads and return a 
        // status accordingly

    return {
        {leftEyePath, HdPrimTypeTokens->mesh},
        {rightEyePath, HdPrimTypeTokens->mesh},
        {mouthPath, HdPrimTypeTokens->basisCurves}
    };
}

HdSceneIndexPrim
UsdImagingSmileyProcedural::GetChildPrim(
    const HdSceneIndexBaseRefPtr &inputScene,
    const SdfPath &childPrimPath)
{
    const TfToken childName = childPrimPath.GetNameToken();

    double eyeSize;
    double smile;
    GfMatrix4d proceduralXform;
    {
        std::lock_guard<std::mutex> lock(_argsMutex);
        eyeSize = _eyeSize;
        smile = _smile;
        proceduralXform = _proceduralXform;
    }

    const HdContainerDataSourceHandle primOriginDs =
        _GetPrimOriginDataSource(_GetProceduralPrimPath());

    if (childName == UsdImagingSmileyTokens->leftEye) {
        return {HdPrimTypeTokens->mesh,
                HdOverlayContainerDataSource::New(
                    primOriginDs,
                    _GetPlacedEyeDataSource(
                        eyeSize, GfVec3d(-0.4, 0.0, 0.4), proceduralXform, _displayColor))};
    }
    if (childName == UsdImagingSmileyTokens->rightEye) {
        return {HdPrimTypeTokens->mesh,
                HdOverlayContainerDataSource::New(
                    primOriginDs,
                    _GetPlacedEyeDataSource(
                        eyeSize, GfVec3d(0.4, 0.0, 0.4), proceduralXform, _displayColor))};
    }
    if (childName == UsdImagingSmileyTokens->mouth) {
        return {HdPrimTypeTokens->basisCurves,
                HdOverlayContainerDataSource::New(
                    primOriginDs,
                    _GetMouthCurveDataSource(smile, proceduralXform, _displayColor))};
    }

    return {};
}

bool
UsdImagingSmileyProcedural::AsyncBegin(bool asyncEnabled)
{
    _asyncEnabled = asyncEnabled;
    return asyncEnabled;
}

AsyncState
UsdImagingSmileyProcedural::AsyncUpdate(
    const ChildPrimTypeMap &previousResult,
    ChildPrimTypeMap *outputPrimTypes,
    HdSceneIndexObserver::DirtiedPrimEntries *outputDirtiedPrims)
{
    
}

class UsdImagingSmileyProceduralPlugin : public HdGpGenerativeProceduralPlugin
{
public:
    UsdImagingSmileyProceduralPlugin() = default;

    HdGpGenerativeProcedural *Construct(
        const SdfPath &proceduralPrimPath) override
    {
        return new UsdImagingSmileyProcedural(proceduralPrimPath);
    }
};

TF_REGISTRY_FUNCTION(TfType)
{
    HdGpGenerativeProceduralPluginRegistry::Define<
        UsdImagingSmileyProceduralPlugin>();
}

PXR_NAMESPACE_CLOSE_SCOPE
