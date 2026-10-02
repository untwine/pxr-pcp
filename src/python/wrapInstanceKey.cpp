//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include <pxr/pcp/pxr.h>
#include <pxr/pcp/instanceKey.h>
#include <pxr/pcp/primIndex.h>

#include <pxr/boost/python/class.hpp>
#include <pxr/boost/python/operators.hpp>
 
PCP_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

namespace {

static size_t
__hash__(const PcpInstanceKey& key)
{
    return TfHash{}(key);
}

} // anonymous namespace 

void
wrapInstanceKey()
{
    class_<PcpInstanceKey>("InstanceKey")
        .def(init<const PcpPrimIndex&>(args("primIndex")))

        .def(self == self)
        .def(self != self)

        .def("__str__", &PcpInstanceKey::GetString)
        .def("__hash__", __hash__)
        ;
}
