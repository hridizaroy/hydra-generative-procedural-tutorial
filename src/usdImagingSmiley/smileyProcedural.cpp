//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "smileyProcedural.h"
#include "smileyMeshGenerator.h"

#include "tokens.h"

#include "pxr/base/gf/math.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/type.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/imaging/hd/primvarSchema.h"
#include "pxr/imaging/hd/primvarsSchema.h"
#include "pxr/imaging/hd/sphereSchema.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hd/xformSchema.h"
#include "pxr/imaging/hdGp/generativeProceduralPlugin.h"
#include "pxr/imaging/hdGp/generativeProceduralPluginRegistry.h"

PXR_NAMESPACE_OPEN_SCOPE

// ---------------------------------------------------------------------------
// UsdImagingSmileyProcedural — scene-reading helpers
// ---------------------------------------------------------------------------

// Reads a plain, non-primvar double value (as published by
// UsdImagingSmileyAdapter) off the procedural prim's own data source,
// clamped to [0, 1].
double
UsdImagingSmileyProcedural::_GetArgValue(
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
SdfPath
UsdImagingSmileyProcedural::_GetTargetPath(
    const HdContainerDataSourceHandle &proceduralPrimDs)
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
GfMatrix4d
UsdImagingSmileyProcedural::_GetXform(
    const HdContainerDataSourceHandle &primDs)
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
double
UsdImagingSmileyProcedural::_GetSphereRadius(
    const HdContainerDataSourceHandle &primDs, double fallback)
{
    if (HdSphereSchema sphereSchema = HdSphereSchema::GetFromParent(primDs)) {
        if (HdDoubleDataSourceHandle radiusDs = sphereSchema.GetRadius()) {
            return radiusDs->GetTypedValue(0);
        }
    }
    return fallback;
}

// ---------------------------------------------------------------------------
// UsdImagingSmileyProcedural
// ---------------------------------------------------------------------------

UsdImagingSmileyProcedural::UsdImagingSmileyProcedural(
    const SdfPath& proceduralPrimPath)
    : HdGpGenerativeProcedural(proceduralPrimPath)
{
}

UsdImagingSmileyProcedural::~UsdImagingSmileyProcedural()
{
    // Cancel in-flight work and join current threads.  Old detached threads
    // already hold their own cancel flag copy and will exit on their own.
    _cancelFlag->store(true);
    if (_eyeThread.joinable())   _eyeThread.join();
    if (_mouthThread.joinable()) _mouthThread.join();
}

HdGpGenerativeProcedural::DependencyMap
UsdImagingSmileyProcedural::UpdateDependencies(
    const HdSceneIndexBaseRefPtr &inputScene)
{
    // eyeSize, smile, and target live on this procedural's own prim, so no
    // dependency declaration is needed for them. But if target is set, the
    // anchor's xform on the target prim is also an input: declare a dependency
    // on it so that moving the target re-cooks this procedural.
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
    // stage. If target is set, anchor at that prim's (local) xform instead of
    // the procedural prim's own; eyeSize/smile still control the face shape
    // independently of the anchor.
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

    VtVec3fArray displayColor{{0.1f, 0.1f, 0.1f}};
    if (HdPrimvarsSchema primvarsSchema = HdPrimvarsSchema::GetFromParent(
            proceduralPrim.dataSource)) {
        if (HdPrimvarSchema primvarSchema =
                primvarsSchema.GetPrimvar(HdTokens->displayColor)) {
            const VtValue value = primvarSchema.GetPrimvarValue()->GetValue(0.0f);
            if (value.IsHolding<VtVec3fArray>()) {
                displayColor = value.UncheckedGet<VtVec3fArray>();
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
                       proceduralXform != _proceduralXform ||
                       displayColor != _displayColor);
        if (argsChanged) {
            _eyeSize = eyeSize;
            _smile = smile;
            _proceduralXform = proceduralXform;
            _displayColor = displayColor;
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

    if (_asyncEnabled) {
        if (argsChanged) {
            // Signal old threads to abandon their results, then detach them.
            // We do NOT wait — this is the key to real-time frame scrubbing.
            // Each thread holds a shared_ptr to its own cancel flag, so
            // detaching is safe even if this procedural is later destroyed.
            _cancelFlag->store(true);
            if (_eyeThread.joinable())   _eyeThread.detach();
            if (_mouthThread.joinable()) _mouthThread.detach();

            // Fresh cancel flag and cook slot for this invocation.
            auto cancelFlag = std::make_shared<std::atomic<bool>>(false);
            _cancelFlag = cancelFlag;
            auto cook = std::make_shared<_PendingCook>();
            _pendingCook = cook;

            const SdfPath originPath = _GetProceduralPrimPath();

            _eyeThread = std::thread(
                [cook, cancelFlag, eyeSize, proceduralXform, displayColor,
                 originPath]()
                {
                    if (cancelFlag->load()) return;
                    cook->leftEye = SmileyMeshGenerator::BuildLeftEyePrim(
                        eyeSize, proceduralXform, displayColor, originPath);
                    if (cancelFlag->load()) return;
                    cook->rightEye = SmileyMeshGenerator::BuildRightEyePrim(
                        eyeSize, proceduralXform, displayColor, originPath);
                    cook->eyesDone.store(true);
                });

            _mouthThread = std::thread(
                [cook, cancelFlag, smile, proceduralXform, displayColor,
                 originPath]()
                {
                    if (cancelFlag->load()) return;
                    cook->mouth = SmileyMeshGenerator::BuildMouthPrim(
                        smile, proceduralXform, displayColor, originPath);
                    cook->mouthDone.store(true);
                });
        }
        return previousResult;
    }

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

    // Serve committed async results for whichever threads have finished,
    // falling back to a synchronous build for the rest.
    if (_asyncEnabled) {
        if (childName == UsdImagingSmileyTokens->leftEye
                && _hasCommittedEyes.load()) {
            std::lock_guard<std::mutex> lock(_committedMutex);
            return _committedLeftEye;
        }
        if (childName == UsdImagingSmileyTokens->rightEye
                && _hasCommittedEyes.load()) {
            std::lock_guard<std::mutex> lock(_committedMutex);
            return _committedRightEye;
        }
        if (childName == UsdImagingSmileyTokens->mouth
                && _hasCommittedMouth.load()) {
            std::lock_guard<std::mutex> lock(_committedMutex);
            return _committedMouth;
        }
    }

    // Synchronous path: read committed args under the args mutex, then build.
    double eyeSize;
    double smile;
    GfMatrix4d proceduralXform;
    VtVec3fArray displayColor;
    {
        std::lock_guard<std::mutex> lock(_argsMutex);
        eyeSize = _eyeSize;
        smile = _smile;
        proceduralXform = _proceduralXform;
        displayColor = _displayColor;
    }

    const SdfPath originPath = _GetProceduralPrimPath();

    if (childName == UsdImagingSmileyTokens->leftEye) {
        return SmileyMeshGenerator::BuildLeftEyePrim(
            eyeSize, proceduralXform, displayColor, originPath);
    }
    if (childName == UsdImagingSmileyTokens->rightEye) {
        return SmileyMeshGenerator::BuildRightEyePrim(
            eyeSize, proceduralXform, displayColor, originPath);
    }
    if (childName == UsdImagingSmileyTokens->mouth) {
        return SmileyMeshGenerator::BuildMouthPrim(
            smile, proceduralXform, displayColor, originPath);
    }

    return {};
}

bool
UsdImagingSmileyProcedural::AsyncBegin(bool asyncEnabled)
{
    _asyncEnabled = asyncEnabled;
    return asyncEnabled;
}

HdGpGenerativeProcedural::AsyncState
UsdImagingSmileyProcedural::AsyncUpdate(
    const ChildPrimTypeMap &previousResult,
    ChildPrimTypeMap *outputPrimTypes,
    HdSceneIndexObserver::DirtiedPrimEntries *outputDirtiedPrims)
{
    const auto cook = _pendingCook;
    if (!cook) {
        return AsyncState::Finished;
    }

    const SdfPath proc = _GetProceduralPrimPath();
    const SdfPath leftEyePath  = proc.AppendChild(UsdImagingSmileyTokens->leftEye);
    const SdfPath rightEyePath = proc.AppendChild(UsdImagingSmileyTokens->rightEye);
    const SdfPath mouthPath    = proc.AppendChild(UsdImagingSmileyTokens->mouth);

    bool anyNew = false;

    // Commit the eye thread's result as soon as it signals done, independently
    // of the mouth thread — this is what ContinuingWithNewChanges enables.
    if (cook->eyesDone.load() && !cook->eyesCommitted.load()) {
        {
            std::lock_guard<std::mutex> lock(_committedMutex);
            _committedLeftEye  = cook->leftEye;
            _committedRightEye = cook->rightEye;
        }
        _hasCommittedEyes.store(true);
        cook->eyesCommitted.store(true);
        anyNew = true;
        if (outputDirtiedPrims) {
            outputDirtiedPrims->emplace_back(
                leftEyePath, HdDataSourceLocatorSet::UniversalSet());
            outputDirtiedPrims->emplace_back(
                rightEyePath, HdDataSourceLocatorSet::UniversalSet());
        }
    }

    // Commit the mouth thread's result independently.
    if (cook->mouthDone.load() && !cook->mouthCommitted.load()) {
        {
            std::lock_guard<std::mutex> lock(_committedMutex);
            _committedMouth = cook->mouth;
        }
        _hasCommittedMouth.store(true);
        cook->mouthCommitted.store(true);
        anyNew = true;
        if (outputDirtiedPrims) {
            outputDirtiedPrims->emplace_back(
                mouthPath, HdDataSourceLocatorSet::UniversalSet());
        }
    }

    // Populate outputPrimTypes with whatever is committed so far.
    if (outputPrimTypes && anyNew) {
        if (_hasCommittedEyes.load()) {
            (*outputPrimTypes)[leftEyePath]  = HdPrimTypeTokens->mesh;
            (*outputPrimTypes)[rightEyePath] = HdPrimTypeTokens->mesh;
        }
        if (_hasCommittedMouth.load()) {
            (*outputPrimTypes)[mouthPath] = HdPrimTypeTokens->basisCurves;
        }
    }

    const bool allDone = cook->eyesDone.load() && cook->mouthDone.load();
    if (allDone) {
        _pendingCook = nullptr;
        if (_eyeThread.joinable())   _eyeThread.join();
        if (_mouthThread.joinable()) _mouthThread.join();
        return anyNew ? AsyncState::FinishedWithNewChanges : AsyncState::Finished;
    }

    return anyNew
        ? AsyncState::ContinuingWithNewChanges
        : AsyncState::Continuing;
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
