//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_IMAGING_USD_IMAGING_SMILEY_PROCEDURAL_H
#define PXR_USD_IMAGING_USD_IMAGING_SMILEY_PROCEDURAL_H

/// \file

#include "pxr/pxr.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/imaging/hdGp/generativeProcedural.h"

#include <mutex>

PXR_NAMESPACE_OPEN_SCOPE

/// A generative procedural that cooks a Smiley prim's eye meshes and mouth
/// basis curve as child prims.
///
/// This performs the same work that UsdImagingSmileyAdapter used to perform
/// directly as subprims; the adapter now instead publishes a
/// generativeProcedural prim (carrying the 'eyeSize' and
/// 'smile' values) that this class is invoked to resolve.
class UsdImagingSmileyProcedural : public HdGpGenerativeProcedural
{
public:
    UsdImagingSmileyProcedural(const SdfPath& proceduralPrimPath);
    ~UsdImagingSmileyProcedural() override;

    DependencyMap UpdateDependencies(
        const HdSceneIndexBaseRefPtr &inputScene) override;

    ChildPrimTypeMap Update(
        const HdSceneIndexBaseRefPtr &inputScene,
        const ChildPrimTypeMap &previousResult,
        const DependencyMap &dirtiedDependencies,
        HdSceneIndexObserver::DirtiedPrimEntries *outputDirtiedPrims) override;

    HdSceneIndexPrim GetChildPrim(
        const HdSceneIndexBaseRefPtr &inputScene,
        const SdfPath &childPrimPath) override;

private:
    // Committed args, written by Update() and read by GetChildPrim(), which
    // per the base class contract may be called concurrently from multiple
    // threads.
    std::mutex _argsMutex;
    double _eyeSize = 0.3;
    double _smile = 1.0;
    GfMatrix4d _proceduralXform{1.0};
    VtVec3fArray _displayColor{{0.1f, 0.1f, 0.1f}};
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
