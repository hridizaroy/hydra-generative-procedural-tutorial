//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdSmileyTokensType::UsdSmileyTokensType() :
    eyeSize("eyeSize", TfToken::Immortal),
    smile("smile", TfToken::Immortal),
    target("target", TfToken::Immortal),
    Smiley("Smiley", TfToken::Immortal),
    allTokens({
        eyeSize,
        smile,
        target,
        Smiley
    })
{
}

TfStaticData<UsdSmileyTokensType> UsdSmileyTokens;

PXR_NAMESPACE_CLOSE_SCOPE
