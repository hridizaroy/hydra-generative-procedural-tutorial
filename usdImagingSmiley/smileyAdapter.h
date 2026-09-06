//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_IMAGING_USD_IMAGING_SMILEY_ADAPTER_H
#define PXR_USD_IMAGING_USD_IMAGING_SMILEY_ADAPTER_H

/// \file

#include "pxr/pxr.h"

#include "pxr/usdImaging/usdImaging/sceneIndexPrimAdapter.h"

PXR_NAMESPACE_OPEN_SCOPE

/// A prim adapter that generates a simple smiley face for Smiley prims.
///
/// Draws two triangle meshes for the eyes and one basis curve for the
/// mouth. The eyes are scaled by the value of the 'smiley:eyeSize'
/// attribute, and the shape of the mouth curve is determined by the value
/// of the 'smiley:smile' attribute: 0 is a frown, 0.5 is a horizontal
/// line, and 1 is a smile.
///
class UsdImagingSmileyAdapter : public UsdImagingSceneIndexPrimAdapter
{
public:
    using BaseAdapter = UsdImagingSceneIndexPrimAdapter;

    TfTokenVector GetImagingSubprims(
        UsdPrim const& prim) override;

    TfToken GetImagingSubprimType(
        UsdPrim const& prim,
        TfToken const& subprim) override;

    HdContainerDataSourceHandle GetImagingSubprimData(
        UsdPrim const& prim,
        TfToken const& subprim,
        const UsdImagingDataSourceStageGlobals &stageGlobals) override;

    HdDataSourceLocatorSet InvalidateImagingSubprim(
        UsdPrim const& prim,
        TfToken const& subprim,
        TfTokenVector const& properties,
        UsdImagingPropertyInvalidationType invalidationType) override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
