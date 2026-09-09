//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_IMAGING_USD_IMAGING_SMILEY_MESH_GENERATOR_H
#define PXR_USD_IMAGING_USD_IMAGING_SMILEY_MESH_GENERATOR_H

/// \file

#include "pxr/pxr.h"

#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/vt/array.h"
#include "pxr/imaging/hd/sceneIndex.h"
#include "pxr/usd/sdf/path.h"

PXR_NAMESPACE_OPEN_SCOPE

/// Builds the Hydra scene index prims for the smiley face geometry: two eye
/// meshes and a mouth basis curve.
///
/// Each Build method returns a complete HdSceneIndexPrim whose data source
/// includes geometry, primvars, an xform (with resetXformStack set), and a
/// prim-origin overlay that ties the synthesized prim back to its parent.
class SmileyMeshGenerator
{
public:
    static HdSceneIndexPrim BuildLeftEyePrim(
        double eyeSize,
        const GfMatrix4d &xform,
        const VtVec3fArray &color,
        const SdfPath &originPath);

    static HdSceneIndexPrim BuildRightEyePrim(
        double eyeSize,
        const GfMatrix4d &xform,
        const VtVec3fArray &color,
        const SdfPath &originPath);

    static HdSceneIndexPrim BuildMouthPrim(
        double smile,
        const GfMatrix4d &xform,
        const VtVec3fArray &color,
        const SdfPath &originPath);
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
