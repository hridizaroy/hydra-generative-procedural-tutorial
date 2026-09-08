//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDSMILEY_TOKENS_H
#define USDSMILEY_TOKENS_H

/// \file usdSmiley/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdSmileyTokensType
///
/// \link UsdSmileyTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdSmileyTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdSmileyTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdSmileyTokens->eyeSize);
/// \endcode
struct UsdSmileyTokensType {
    USDSMILEY_API UsdSmileyTokensType();
    /// \brief "eyeSize"
    /// 
    /// UsdSmileySmiley
    const TfToken eyeSize;
    /// \brief "smile"
    /// 
    /// UsdSmileySmiley
    const TfToken smile;
    /// \brief "target"
    /// 
    /// UsdSmileySmiley
    const TfToken target;
    /// \brief "Smiley"
    /// 
    /// Schema identifer and family for UsdSmileySmiley
    const TfToken Smiley;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdSmileyTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdSmileyTokensType
extern USDSMILEY_API TfStaticData<UsdSmileyTokensType> UsdSmileyTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
