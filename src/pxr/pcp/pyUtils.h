//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_PCP_PY_UTILS_H
#define PXR_USD_PCP_PY_UTILS_H

#include <pxr/boost/python/dict.hpp>

#include <pxr/pcp/pxr.h>
#include <pxr/pcp/api.h>
#include <pxr/pcp/types.h>

#include <string>

PCP_NAMESPACE_OPEN_SCOPE

/// Convert the Python dictionary \p dict to a PcpVariantFallbackMap
/// object and return it via \p result, returning true if successful.
PCP_API
bool
PcpVariantFallbackMapFromPython(
    const pxr_boost::python::dict& dict,
    PcpVariantFallbackMap *result);

PCP_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_PCP_PY_UTILS_H
