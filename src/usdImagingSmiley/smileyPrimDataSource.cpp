//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "smileyPrimDataSource.h"

#include "tokens.h"

#include "usdsmileygenproc/usdSmiley/smiley.h"

#include "pxr/base/tf/token.h"
#include "pxr/imaging/hd/overlayContainerDataSource.h"
#include "pxr/imaging/hd/primvarSchema.h"
#include "pxr/imaging/hd/primvarsSchema.h"
#include "pxr/imaging/hd/retainedDataSource.h"
#include "pxr/imaging/hdGp/generativeProcedural.h"
#include "pxr/usd/usd/relationship.h"
#include "pxr/usdImaging/usdImaging/dataSourceAttribute.h"

#include <algorithm>

PXR_NAMESPACE_OPEN_SCOPE

SmileyPrimDataSource::SmileyPrimDataSource(
    const UsdPrim &prim,
    const UsdImagingDataSourceStageGlobals &stageGlobals)
    : _prim(prim)
    , _stageGlobals(stageGlobals)
{
}

TfTokenVector
SmileyPrimDataSource::GetNames()
{
    TfTokenVector names = {
        HdPrimvarsSchema::GetSchemaToken(),
        UsdImagingSmileyTokens->smileyEyeSize,
        UsdImagingSmileyTokens->smileySmile,
        UsdImagingSmileyTokens->smileyTarget,
    };

    const HdContainerDataSourceHandle base =
        UsdImagingDataSourcePrim::New(_prim.GetPath(), _prim, _stageGlobals);
    for (const TfToken &name : base->GetNames()) {
        if (std::find(names.begin(), names.end(), name) == names.end()) {
            names.push_back(name);
        }
    }
    return names;
}

HdDataSourceBaseHandle
SmileyPrimDataSource::Get(const TfToken &name)
{
    if (name == HdPrimvarsSchema::GetSchemaToken()) {
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
        // Overlay our proceduralType primvar on top of the base prim's
        // primvars so that USD-authored primvars (e.g. displayColor) are
        // preserved alongside it.
        const HdContainerDataSourceHandle procTypePrimvars =
            HdPrimvarsSchema::BuildRetained(1, procTypeTokens, procTypeValues);
        const HdContainerDataSourceHandle basePrimvars =
            HdContainerDataSource::Cast(
                UsdImagingDataSourcePrim::New(
                    _prim.GetPath(), _prim, _stageGlobals)
                        ->Get(HdPrimvarsSchema::GetSchemaToken()));
        if (basePrimvars) {
            return HdOverlayContainerDataSource::New(procTypePrimvars, basePrimvars);
        }
        return procTypePrimvars;
    }

    if (name == UsdImagingSmileyTokens->smileyEyeSize) {
        return UsdImagingDataSourceAttribute<double>::New(
            _prim.GetAttribute(UsdImagingSmileyTokens->smileyEyeSize),
            _stageGlobals);
    }
    if (name == UsdImagingSmileyTokens->smileySmile) {
        return UsdImagingDataSourceAttribute<double>::New(
            _prim.GetAttribute(UsdImagingSmileyTokens->smileySmile),
            _stageGlobals);
    }
    if (name == UsdImagingSmileyTokens->smileyTarget) {
        SdfPathVector targets;
        UsdSmileySmiley(_prim).GetTargetRel().GetForwardedTargets(&targets);
        return HdRetainedTypedSampledDataSource<VtArray<SdfPath>>::New(
            VtArray<SdfPath>(targets.begin(), targets.end()));
    }

    // Delegate everything else (xform, purpose, visibility, …) to the base.
    return UsdImagingDataSourcePrim::New(
        _prim.GetPath(), _prim, _stageGlobals)->Get(name);
}

PXR_NAMESPACE_CLOSE_SCOPE
