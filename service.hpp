#ifndef _SERVICE_HPP
#define _SERVICE_HPP

#include "so.hpp"

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
    typedef std::shared_ptr<Service> ServicePtr_t;
    typedef std::map<std::string, ServicePtr_t> ServiceMap_t;

    Service() = delete;
    Service(const Service& other) = delete;
    Service(Service&& other) = delete;
    Service(const jsonx::json &meta, SharedObject::Ptr_t so = nullptr);
    ~Service();

    static bool is_defined(const std::string &name) {
        return service_map.contains(name);
    }

    static ServicePtr_t lookup(const std::string &name) {
        auto iter = service_map.find(name);
        return (iter != service_map.end()) ? iter->second : nullptr;
    }

    static const Service& lookup_service(const std::string &name) {
        auto ptr = lookup(name);
        if (!ptr)
            throw ServiceException("Service not found: " + name);
        return *ptr; 
    }

    static ServicePtr_t create(jsonx::json meta, SharedObject::Ptr_t so = nullptr);

    static const Service& create_service(jsonx::json meta, SharedObject::Ptr_t so = nullptr);

    static bool remove_service(const std::string &name) {
        return service_map.erase(name);
    }

    std::string get_name() const { return meta["name"]; }

private:
    static ServiceMap_t service_map;

    jsonx::json         meta;
    SharedObject::Ptr_t so;
};



#endif // _SERVICE_HPP //
