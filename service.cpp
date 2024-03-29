#include "service.hpp"

#include <sstream>

using namespace std;
using namespace jsonx;

Service::Map_t Service::container;

Service::Service(const json &_meta, SharedObject::Ptr_t _so, const service_reg_t& _service_reg):
    meta{_meta}, so{_so}, service_reg{_service_reg}
{
}

Service::~Service()
{
}

Service::Ptr_t Service::create(json meta, SharedObject::Ptr_t so, const service_reg_t& service_reg)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Service(meta, so, service_reg)).first->second;
}

const Service& Service::create_service(json meta, SharedObject::Ptr_t so,
                                       const service_reg_t& service_reg) {
    string name = meta["name"];
    if (container.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *container.emplace(name, new Service(meta, so, service_reg)).first->second;
}

