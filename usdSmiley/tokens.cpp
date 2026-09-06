//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/usd/usdSmiley/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdSmileyTokensType::UsdSmileyTokensType() :
    smileyEyeSize("smiley:eyeSize", TfToken::Immortal),
    smileySmile("smiley:smile", TfToken::Immortal),
    smileyTarget("smiley:target", TfToken::Immortal),
    Smiley("Smiley", TfToken::Immortal),
    allTokens({
        smileyEyeSize,
        smileySmile,
        smileyTarget,
        Smiley
    })
{
}

TfStaticData<UsdSmileyTokensType> UsdSmileyTokens;

PXR_NAMESPACE_CLOSE_SCOPE
