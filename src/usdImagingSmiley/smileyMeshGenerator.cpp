//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "smileyMeshGenerator.h"

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/vt/array.h"
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
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hd/xformSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

namespace {

// ---------------------------------------------------------------------------
// Geometry primitives
// ---------------------------------------------------------------------------

const VtVec3fArray&
_GetUnitTrianglePoints()
{
    static const VtVec3fArray points = {
        { 0.0f,  0.0f,  0.5f},
        {-0.5f,  0.0f, -0.5f},
        { 0.5f,  0.0f, -0.5f}
    };
    return points;
}

const VtIntArray&
_GetUnitTriangleFaceVertexCounts()
{
    static const VtIntArray counts = {3};
    return counts;
}

const VtIntArray&
_GetUnitTriangleFaceVertexIndices()
{
    static const VtIntArray indices = {0, 1, 2};
    return indices;
}

// ---------------------------------------------------------------------------
// Data source builders
// ---------------------------------------------------------------------------

HdDataSourceBaseHandle
_BuildDisplayColorDs(const VtVec3fArray &color)
{
    return HdPrimvarSchema::Builder()
        .SetPrimvarValue(
            HdRetainedTypedSampledDataSource<VtVec3fArray>::New(color))
        .SetInterpolation(
            HdPrimvarSchema::BuildInterpolationDataSource(
                HdPrimvarSchemaTokens->constant))
        .Build();
}

HdContainerDataSourceHandle
_BuildEyeMeshDs(double eyeSize, const VtVec3fArray &color)
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
        _BuildDisplayColorDs(color)
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

// Records 'originPath' as this synthesized child's prim origin, so that
// selecting the parent Smiley prim also selects/highlights the generated prims.
HdContainerDataSourceHandle
_BuildPrimOriginDs(const SdfPath &originPath)
{
    return HdRetainedContainerDataSource::New(
        HdPrimOriginSchemaTokens->primOrigin,
        HdRetainedContainerDataSource::New(
            HdPrimOriginSchemaTokens->scenePath,
            HdRetainedTypedSampledDataSource<HdPrimOriginSchema::OriginPath>::New(
                HdPrimOriginSchema::OriginPath(originPath))));
}

// Places an eye mesh at 'center' via an xform overlay. 'proceduralXform' is
// the procedural prim's already-inherited transform; resetXformStack prevents
// downstream re-composition.
HdContainerDataSourceHandle
_BuildPlacedEyeDs(
    double eyeSize, const GfVec3d &center,
    const GfMatrix4d &proceduralXform, const VtVec3fArray &color)
{
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
        _BuildEyeMeshDs(eyeSize, color));
}

// Builds the mouth basis curve. 'smile' in [0,1] controls curvature:
// 0.5 = flat, 1.0 = smile, 0.0 = frown.
HdContainerDataSourceHandle
_BuildMouthCurveDs(
    double smile, const GfMatrix4d &proceduralXform, const VtVec3fArray &color)
{
    constexpr int numPoints = 11;
    constexpr float halfWidth = 0.5f;
    constexpr float baseZ = -0.4f;
    constexpr float amplitude = 0.35f;
    const float curveFactor = static_cast<float>(2.0 * smile - 1.0);

    VtVec3fArray points(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        const float t = static_cast<float>(i) / (numPoints - 1);
        const float x = -halfWidth + t * (2.0f * halfWidth);
        const float u = x / halfWidth;
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
        _BuildDisplayColorDs(color)
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

} // namespace

// ---------------------------------------------------------------------------
// SmileyMeshGenerator
// ---------------------------------------------------------------------------

HdSceneIndexPrim
SmileyMeshGenerator::BuildLeftEyePrim(
    double eyeSize, const GfMatrix4d &xform,
    const VtVec3fArray &color, const SdfPath &originPath)
{
    return {HdPrimTypeTokens->mesh,
            HdOverlayContainerDataSource::New(
                _BuildPrimOriginDs(originPath),
                _BuildPlacedEyeDs(eyeSize, GfVec3d(-0.4, 0.0, 0.4), xform, color))};
}

HdSceneIndexPrim
SmileyMeshGenerator::BuildRightEyePrim(
    double eyeSize, const GfMatrix4d &xform,
    const VtVec3fArray &color, const SdfPath &originPath)
{
    return {HdPrimTypeTokens->mesh,
            HdOverlayContainerDataSource::New(
                _BuildPrimOriginDs(originPath),
                _BuildPlacedEyeDs(eyeSize, GfVec3d(0.4, 0.0, 0.4), xform, color))};
}

HdSceneIndexPrim
SmileyMeshGenerator::BuildMouthPrim(
    double smile, const GfMatrix4d &xform,
    const VtVec3fArray &color, const SdfPath &originPath)
{
    return {HdPrimTypeTokens->basisCurves,
            HdOverlayContainerDataSource::New(
                _BuildPrimOriginDs(originPath),
                _BuildMouthCurveDs(smile, xform, color))};
}

PXR_NAMESPACE_CLOSE_SCOPE
