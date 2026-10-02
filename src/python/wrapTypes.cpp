//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include <pxr/pcp/pxr.h>
#include <pxr/pcp/types.h>
#include <pxr/tf/pyEnum.h>

PCP_NAMESPACE_USING_DIRECTIVE

void
wrapTypes()
{
    TfPyWrapEnum<PcpArcType>();
}
