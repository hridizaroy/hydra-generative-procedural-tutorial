//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_IMAGING_USD_IMAGING_SMILEY_PRIM_DATA_SOURCE_H
#define PXR_USD_IMAGING_USD_IMAGING_SMILEY_PRIM_DATA_SOURCE_H

/// \file

#include "pxr/pxr.h"

#include "pxr/imaging/hd/dataSource.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usdImaging/usdImaging/dataSourcePrim.h"

PXR_NAMESPACE_OPEN_SCOPE

/// Data source for the generativeProcedural prim published by
/// UsdImagingSmileyAdapter.  Exposes the procedural-type primvar, the
/// smileyEyeSize / smileySmile attributes, and the resolved target path, then
/// delegates everything else (xform, visibility, purpose, …) to the base
/// UsdImagingDataSourcePrim.
class SmileyPrimDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(SmileyPrimDataSource);

    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken &name) override;

private:
    SmileyPrimDataSource(
        const UsdPrim &prim,
        const UsdImagingDataSourceStageGlobals &stageGlobals);

    UsdPrim _prim;
    const UsdImagingDataSourceStageGlobals &_stageGlobals;
};

HD_DECLARE_DATASOURCE_HANDLES(SmileyPrimDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif
