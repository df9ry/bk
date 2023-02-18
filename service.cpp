#include "service.hpp"

using namespace std;
using namespace jsonx;

Service::ServiceMap_t Service::service_map;

Service::Service(const json &_meta, SharedObject::Ptr_t _so):
    meta{_meta}, so{_so}
{
}

Service::~Service()
{
}

Service::ServicePtr_t Service::create(json meta, SharedObject::Ptr_t _so)    
{
    string name = meta["name"];
    if (service_map.contains(name))
        return nullptr;
    return service_map.emplace(name, new Service(meta, _so)).first->second;
}

const Service& Service::create_service(json meta, SharedObject::Ptr_t _so) {
    string name = meta["name"];
    if (service_map.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *service_map.emplace(name, new Service(meta, _so)).first->second;
}
