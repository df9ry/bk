#ifndef _SERVICE_HPP
#define _SERVICE_HPP

#include "so.hpp"

#include "bk/session.h"

#include <stdexcept>
#include <string>
#include <map>
#include <memory>
#include <exception>

#include <jsonx.hpp>

class ServiceException: public std::runtime_error
{
public:
    ServiceException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Service {
public:
    typedef std::shared_ptr<Service> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    static Map_t container;

    Service(const jsonx::json &meta, SharedObject::Ptr_t so, const session_admin_t* sap);
    ~Service();

    Service() = delete;
    Service(const Service& other) = delete;
    Service(Service&& other) = delete;

    static bool is_defined(const std::string &name) {
        return container.contains(name);
    }

    static Ptr_t lookup(const std::string &name) {
        auto iter = container.find(name);
        return (iter != container.end()) ? iter->second : nullptr;
    }

    static const Service& lookup_service(const std::string &name) {
        auto ptr = lookup(name);
        if (!ptr)
            throw ServiceException("Service not found: " + name);
        return *ptr; 
    }

    static Ptr_t create(jsonx::json meta, SharedObject::Ptr_t so, const session_admin_t* sap);

    static const Service& create_service(jsonx::json meta, SharedObject::Ptr_t so,
                                         session_admin_t* sap);

    static bool remove_service(const std::string &name) {
        return container.erase(name);
    }

    const std::string get_name() const { return meta["name"]; }
    const SharedObject::Ptr_t get_plugin() const { return so; }

private:
    jsonx::json         meta;
    SharedObject::Ptr_t so;
    session_admin_t     session_admin_ifc{};
};

#endif // _SERVICE_HPP //
