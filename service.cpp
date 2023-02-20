#include "service.hpp"

using namespace std;
using namespace jsonx;

Plugin::Map_t Plugin::container;

Plugin::Plugin(const json &_meta, SharedObject::Ptr_t _so, const send_t* _endpoint):
    meta{_meta}, so{_so}, endpoint{*_endpoint}
{
}

Plugin::~Plugin()
{
}

Plugin::Ptr_t Plugin::create(json meta, SharedObject::Ptr_t _so, const send_t* _endpoint)    
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Plugin(meta, _so, _endpoint)).first->second;
}

const Plugin& Plugin::create_service(json meta, SharedObject::Ptr_t _so, const send_t* _endpoint) {
    string name = meta["name"];
    if (container.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *container.emplace(name, new Plugin(meta, _so, _endpoint)).first->second;
}
