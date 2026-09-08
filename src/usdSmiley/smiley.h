//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDSMILEY_GENERATED_SMILEY_H
#define USDSMILEY_GENERATED_SMILEY_H

/// \file usdSmiley/smiley.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usdGeom/boundable.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "./tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// SMILEY                                                                     //
// -------------------------------------------------------------------------- //

/// \class UsdSmileySmiley
///
/// A very simple example prim type representing a smiley face.
/// 
/// It is drawn by the UsdImaging 2.0 adapter as two triangle meshes (for the
/// eyes) and a basis curve (for the mouth), procedurally generated from the
/// attributes below.
/// 
///
class UsdSmileySmiley : public UsdGeomBoundable
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

    /// Construct a UsdSmileySmiley on UsdPrim \p prim .
    /// Equivalent to UsdSmileySmiley::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdSmileySmiley(const UsdPrim& prim=UsdPrim())
        : UsdGeomBoundable(prim)
    {
    }

    /// Construct a UsdSmileySmiley on the prim held by \p schemaObj .
    /// Should be preferred over UsdSmileySmiley(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdSmileySmiley(const UsdSchemaBase& schemaObj)
        : UsdGeomBoundable(schemaObj)
    {
    }

    /// Destructor.
    USDSMILEY_API
    virtual ~UsdSmileySmiley();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDSMILEY_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdSmileySmiley holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdSmileySmiley(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDSMILEY_API
    static UsdSmileySmiley
    Get(const UsdStagePtr &stage, const SdfPath &path);

    /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
    /// is defined (according to UsdPrim::IsDefined()) on this stage.
    ///
    /// If a prim adhering to this schema at \p path is already defined on this
    /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
    /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
    /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
    /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
    /// current EditTarget for any nonexistent, or existing but not \a Defined
    /// ancestors.
    ///
    /// The given \a path must be an absolute prim path that does not contain
    /// any variant selections.
    ///
    /// If it is impossible to author any of the necessary PrimSpecs, (for
    /// example, in case \a path cannot map to the current UsdEditTarget's
    /// namespace) issue an error and return an invalid \a UsdPrim.
    ///
    /// Note that this method may return a defined prim whose typeName does not
    /// specify this schema class, in case a stronger typeName opinion overrides
    /// the opinion at the current EditTarget.
    ///
    USDSMILEY_API
    static UsdSmileySmiley
    Define(const UsdStagePtr &stage, const SdfPath &path);

protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDSMILEY_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDSMILEY_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDSMILEY_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // EYESIZE 
    // --------------------------------------------------------------------- //
    /// Size of the smiley's eye triangles, from 0 (no eyes) to
    /// 1 (largest). Values are clamped to [0, 1].
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `double eyeSize = 0.3` |
    /// | C++ Type | double |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Double |
    USDSMILEY_API
    UsdAttribute GetEyeSizeAttr() const;

    /// See GetEyeSizeAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDSMILEY_API
    UsdAttribute CreateEyeSizeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SMILE 
    // --------------------------------------------------------------------- //
    /// Shape of the smiley's mouth. 0 is a frown, 0.5 is a
    /// horizontal line, and 1 is a proper smile. Values are clamped
    /// to [0, 1].
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `double smile = 1` |
    /// | C++ Type | double |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Double |
    USDSMILEY_API
    UsdAttribute GetSmileAttr() const;

    /// See GetSmileAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDSMILEY_API
    UsdAttribute CreateSmileAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // TARGET 
    // --------------------------------------------------------------------- //
    /// Optional single-target relationship. When set to a
    /// UsdGeomSphere, the smiley face is drawn at that sphere's
    /// location instead of at this prim's own transform.
    ///
    USDSMILEY_API
    UsdRelationship GetTargetRel() const;

    /// See GetTargetRel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDSMILEY_API
    UsdRelationship CreateTargetRel() const;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
