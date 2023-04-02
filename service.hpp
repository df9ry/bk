#ifndef _SERVICE_HPP
#define _SERVICE_HPP

#include "so.hpp"
#include "bk/service.h"

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

    Service(const jsonx::json &meta, SharedObject::Ptr_t so, const service_t& service_ifc);
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

    static Ptr_t create(jsonx::json meta, SharedObject::Ptr_t so, const service_t& service_ifc);

    static const Service& create_service(jsonx::json meta, SharedObject::Ptr_t so,
                                         const service_t& service_ifc);

    static bool remove_service(const std::string &name) {
        return container.erase(name);
    }

    const service_t service_ifc;

    const std::string get_name() const { return meta["name"]; }
    const SharedObject::Ptr_t get_plugin() const { return so; }

private:
    jsonx::json         meta;
    SharedObject::Ptr_t so;
};

#endif // _SERVICE_HPP //
