//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include <pxr/pcp/pxr.h>
#include <pxr/pcp/cache.h>
#include <pxr/pcp/changes.h>

#include <pxr/sdf/changeList.h>
#include <pxr/sdf/notice.h>

#include <pxr/tf/pyResultConversions.h>
#include <pxr/tf/weakBase.h>
#include <pxr/boost/python.hpp>

PCP_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

namespace {

class Pcp_PyTestChangeProcessor
    : public TfWeakBase
{
public:
    Pcp_PyTestChangeProcessor(const PcpCache* cache)
        : _cache(cache)
    {
    }

    Pcp_PyTestChangeProcessor(const Pcp_PyTestChangeProcessor&) = delete;

    Pcp_PyTestChangeProcessor&
    operator=(const Pcp_PyTestChangeProcessor&) = delete;

    void Enter()
    {
        _layerChangedNoticeKey = TfNotice::Register(
            TfCreateWeakPtr(this),
            &Pcp_PyTestChangeProcessor::_HandleLayerDidChange);
    }

    void Exit(const object&, const object&, const object&)
    {
        TfNotice::Revoke(_layerChangedNoticeKey);
        _changes = PcpChanges();
    }

    SdfPathVector GetSignificantChanges() const
    {
        TF_FOR_ALL(it, _changes.GetCacheChanges()) {
            if (it->first == _cache) {
                return SdfPathVector(
                    it->second.didChangeSignificantly.begin(), 
                    it->second.didChangeSignificantly.end());
            }
        }
        return SdfPathVector();
    }

    SdfPathVector GetSpecChanges() const
    {
        TF_FOR_ALL(it, _changes.GetCacheChanges()) {
            if (it->first == _cache) {
                return SdfPathVector(
                    it->second.didChangeSpecs.begin(), 
                    it->second.didChangeSpecs.end());
            }
        }
        return SdfPathVector();
    }

    SdfPathVector GetPrimChanges() const
    {
        TF_FOR_ALL(it, _changes.GetCacheChanges()) {
            if (it->first == _cache) {
                return SdfPathVector(
                    it->second.didChangePrims.begin(), 
                    it->second.didChangePrims.end());
            }
        }
        return SdfPathVector();
    }

private:
    void _HandleLayerDidChange(const SdfNotice::LayersDidChange& n)
    {
        _changes.DidChange(_cache, n.GetChangeListVec());
        _changes.Apply();
    }

private:
    const PcpCache* _cache;
    TfNotice::Key _layerChangedNoticeKey;
    PcpChanges _changes;
};

} // anonymous namespace 

void
wrapTestChangeProcessor()
{
    typedef Pcp_PyTestChangeProcessor This;
    typedef TfWeakPtr<Pcp_PyTestChangeProcessor> ThisPtr;

    class_<This, ThisPtr, noncopyable>
        ("_TestChangeProcessor", init<PcpCache*>())
        .def("__enter__", &This::Enter, return_self<>())
        .def("__exit__", &This::Exit)

        .def("GetSignificantChanges", 
            &This::GetSignificantChanges,
            return_value_policy<TfPySequenceToList>())
        .def("GetSpecChanges", 
            &This::GetSpecChanges,
            return_value_policy<TfPySequenceToList>())
        .def("GetPrimChanges", 
            &This::GetPrimChanges,
            return_value_policy<TfPySequenceToList>())
        ;
}
