//
// Copyright 2017 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDSMILEY_API_H
#define USDSMILEY_API_H

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDSMILEY_API
#   define USDSMILEY_API_TEMPLATE_CLASS(...)
#   define USDSMILEY_API_TEMPLATE_STRUCT(...)
#   define USDSMILEY_LOCAL
#else
#   if defined(USDSMILEY_EXPORTS)
#       define USDSMILEY_API ARCH_EXPORT
#       define USDSMILEY_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDSMILEY_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDSMILEY_API ARCH_IMPORT
#       define USDSMILEY_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDSMILEY_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDSMILEY_LOCAL ARCH_HIDDEN
#endif

#endif
