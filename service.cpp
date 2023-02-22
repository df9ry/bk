#include "service.hpp"

using namespace std;
using namespace jsonx;

Service::Map_t Service::container;

Service::Service(const json &_meta, SharedObject::Ptr_t _so, const session_admin_t* _sap):
    meta{_meta}, so{_so}, session_admin_ifc{*_sap}
{
}

Service::~Service()
{
}

Service::Ptr_t Service::create(json meta, SharedObject::Ptr_t _so, const session_admin_t* _sap)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Service(meta, _so, _sap)).first->second;
}

const Service& Service::create_service(json meta, SharedObject::Ptr_t _so, session_admin_t* _sap) {
    string name = meta["name"];
    if (container.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *container.emplace(name, new Service(meta, _so, _sap)).first->second;
}

bk_error_t Service::start() {

    //TODO: Use debug interface
    cout << "[i] Start service \"" << get_name() << "\"" << endl;
    return BK_ERC_OK;
}

bk_error_t Service::stop() {
    //TODO: Use debug interface
    cout << "[i] Stop service \"" << get_name() << "\"" << endl;
    return BK_ERC_OK;
}
