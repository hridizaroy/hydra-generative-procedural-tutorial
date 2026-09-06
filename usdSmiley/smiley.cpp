//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/usd/usdSmiley/smiley.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdSmileySmiley,
        TfType::Bases< UsdGeomBoundable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("Smiley")
    // to find TfType<UsdSmileySmiley>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdSmileySmiley>("Smiley");
}

/* virtual */
UsdSmileySmiley::~UsdSmileySmiley()
{
}

/* static */
UsdSmileySmiley
UsdSmileySmiley::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdSmileySmiley();
    }
    return UsdSmileySmiley(stage->GetPrimAtPath(path));
}

/* static */
UsdSmileySmiley
UsdSmileySmiley::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("Smiley");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdSmileySmiley();
    }
    return UsdSmileySmiley(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdSmileySmiley::_GetSchemaKind() const
{
    return UsdSmileySmiley::schemaKind;
}

/* static */
const TfType &
UsdSmileySmiley::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdSmileySmiley>();
    return tfType;
}

/* static */
bool 
UsdSmileySmiley::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdSmileySmiley::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdSmileySmiley::GetSmileyEyeSizeAttr() const
{
    return GetPrim().GetAttribute(UsdSmileyTokens->smileyEyeSize);
}

UsdAttribute
UsdSmileySmiley::CreateSmileyEyeSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdSmileyTokens->smileyEyeSize,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdSmileySmiley::GetSmileySmileAttr() const
{
    return GetPrim().GetAttribute(UsdSmileyTokens->smileySmile);
}

UsdAttribute
UsdSmileySmiley::CreateSmileySmileAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdSmileyTokens->smileySmile,
                       SdfValueTypeNames->Double,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdSmileySmiley::GetSmileyTargetRel() const
{
    return GetPrim().GetRelationship(UsdSmileyTokens->smileyTarget);
}

UsdRelationship
UsdSmileySmiley::CreateSmileyTargetRel() const
{
    return GetPrim().CreateRelationship(UsdSmileyTokens->smileyTarget,
                       /* custom = */ false);
}

namespace {
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector& left,const TfTokenVector& right)
{
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
}
}

/*static*/
const TfTokenVector&
UsdSmileySmiley::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdSmileyTokens->smileyEyeSize,
        UsdSmileyTokens->smileySmile,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomBoundable::GetSchemaAttributeNames(true),
            localNames);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
