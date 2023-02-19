#include "service.hpp"

using namespace std;
using namespace jsonx;

Service::Map_t Service::service_map;

Service::Service(const json &_meta, SharedObject::Ptr_t _so, const send_t* _endpoint):
    meta{_meta}, so{_so}, endpoint{*_endpoint}
{
}

Service::~Service()
{
}

Service::Ptr_t Service::create(json meta, SharedObject::Ptr_t _so, const send_t* _endpoint)    
{
    string name = meta["name"];
    if (service_map.contains(name))
        return nullptr;
    return service_map.emplace(name, new Service(meta, _so, _endpoint)).first->second;
}

const Service& Service::create_service(json meta, SharedObject::Ptr_t _so, const send_t* _endpoint) {
    string name = meta["name"];
    if (service_map.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *service_map.emplace(name, new Service(meta, _so, _endpoint)).first->second;
}
