//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "smileyAdapter.h"

#include "tokens.h"

#include "usdsmileygenproc/usdSmiley/smiley.h"
#include "pxr/usd/usd/relationship.h"

#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/type.h"
#include "pxr/imaging/hd/dataSourceLocator.h"
#include "pxr/imaging/hd/overlayContainerDataSource.h"
#include "pxr/imaging/hd/primvarSchema.h"
#include "pxr/imaging/hd/primvarsSchema.h"
#include "pxr/imaging/hd/retainedDataSource.h"
#include "pxr/imaging/hdGp/generativeProcedural.h"
#include "pxr/usdImaging/usdImaging/dataSourceAttribute.h"
#include "pxr/usdImaging/usdImaging/dataSourcePrim.h"

#include <algorithm>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    using Adapter = UsdImagingSmileyAdapter;
    const TfType type =
        TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter>>();
    type.SetFactory<UsdImagingPrimAdapterFactory<Adapter>>();
}

TfTokenVector
UsdImagingSmileyAdapter::GetImagingSubprims(
    UsdPrim const& prim)
{
    return { TfToken() };
}

TfToken
UsdImagingSmileyAdapter::GetImagingSubprimType(
    UsdPrim const& prim,
    TfToken const& subprim)
{
    if (subprim.IsEmpty()) {
        return HdGpGenerativeProceduralTokens->generativeProcedural;
    }

    TF_CODING_ERROR("Unsupported subprim '%s'", subprim.GetText());
    return {};
}

// Publishes this prim as a generative procedural (see
// UsdImagingSmileyProcedural), passing 'eyeSize' and 'smile'
// through as plain data source values for the procedural to read.
HdContainerDataSourceHandle
UsdImagingSmileyAdapter::GetImagingSubprimData(
    UsdPrim const& prim,
    TfToken const& subprim,
    const UsdImagingDataSourceStageGlobals &stageGlobals)
{
    if (!subprim.IsEmpty()) {
        TF_CODING_ERROR("Unsupported subprim '%s'", subprim.GetText());
        return nullptr;
    }

    static const TfToken procTypeTokens[] = {
        HdGpGenerativeProceduralTokens->proceduralType
    };
    const HdDataSourceBaseHandle procTypeValues[] = {
        HdPrimvarSchema::Builder()
            .SetPrimvarValue(
                HdRetainedTypedSampledDataSource<TfToken>::New(
                    UsdImagingSmileyTokens->smileyProceduralType))
            .SetInterpolation(
                HdPrimvarSchema::BuildInterpolationDataSource(
                    HdPrimvarSchemaTokens->constant))
            .Build()
    };

    SdfPathVector targets;
    UsdSmileySmiley(prim).GetTargetRel().GetForwardedTargets(&targets);
    const VtArray<SdfPath> targetArray(targets.begin(), targets.end());

    return HdOverlayContainerDataSource::New(
        HdRetainedContainerDataSource::New(
            HdPrimvarsSchema::GetSchemaToken(),
            HdPrimvarsSchema::BuildRetained(1, procTypeTokens, procTypeValues),

            UsdImagingSmileyTokens->smileyEyeSize,
            UsdImagingDataSourceAttribute<double>::New(
                prim.GetAttribute(UsdImagingSmileyTokens->smileyEyeSize),
                stageGlobals),

            UsdImagingSmileyTokens->smileySmile,
            UsdImagingDataSourceAttribute<double>::New(
                prim.GetAttribute(UsdImagingSmileyTokens->smileySmile),
                stageGlobals),

            UsdImagingSmileyTokens->smileyTarget,
            HdRetainedTypedSampledDataSource<VtArray<SdfPath>>::New(
                targetArray)),
        UsdImagingDataSourcePrim::New(prim.GetPath(), prim, stageGlobals));
}

HdDataSourceLocatorSet
UsdImagingSmileyAdapter::InvalidateImagingSubprim(
    UsdPrim const& prim,
    TfToken const& subprim,
    TfTokenVector const& properties,
    UsdImagingPropertyInvalidationType invalidationType)
{
    const auto changed = [&properties](TfToken const& propName) {
        return std::find(properties.begin(), properties.end(), propName)
            != properties.end();
    };

    if (changed(UsdImagingSmileyTokens->smileyEyeSize) ||
        changed(UsdImagingSmileyTokens->smileySmile) ||
        changed(UsdImagingSmileyTokens->smileyTarget)) {
        return HdDataSourceLocatorSet::UniversalSet();
    }

    return UsdImagingDataSourcePrim::Invalidate(
        prim, subprim, properties, invalidationType);
}

PXR_NAMESPACE_CLOSE_SCOPE
