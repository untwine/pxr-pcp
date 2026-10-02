//
// Copyright 2020 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include <pxr/pcp/pxr.h>
#include <pxr/pcp/dynamicFileFormatInterface.h>

PCP_NAMESPACE_OPEN_SCOPE

PcpDynamicFileFormatInterface::~PcpDynamicFileFormatInterface() = default;

bool 
PcpDynamicFileFormatInterface::CanFieldChangeAffectFileFormatArguments(
    const TfToken &field,
    const VtValue &oldValue,
    const VtValue &newValue,
    const VtValue &dependencyContextData) const
{
    return true;
}

bool 
PcpDynamicFileFormatInterface::
CanAttributeDefaultValueChangeAffectFileFormatArguments(
    const TfToken &attributeName,
    const VtValue &oldValue,
    const VtValue &newValue,
    const VtValue &dependencyContextData) const
{
    return true;
}

PCP_NAMESPACE_CLOSE_SCOPE
