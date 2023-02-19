#ifndef _SERVICE_HPP
#define _SERVICE_HPP

#include "so.hpp"

#include <stdexcept>
#include <string>
#include <map>
#include <memory>
#include <exception>

#include <jsonx.hpp>
#include "bk/send.h"

class ServiceException: public std::runtime_error
{
public:
    ServiceException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Service {
public:
    typedef std::shared_ptr<Service> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    Service(
        const jsonx::json &meta, 
        SharedObject::Ptr_t so,
        const send_t* endpoint);
    ~Service();

    Service() = delete;
    Service(const Service& other) = delete;
    Service(Service&& other) = delete;

    static bool is_defined(const std::string &name) {
        return service_map.contains(name);
    }

    static Ptr_t lookup(const std::string &name) {
        auto iter = service_map.find(name);
        return (iter != service_map.end()) ? iter->second : nullptr;
    }

    static const Service& lookup_service(const std::string &name) {
        auto ptr = lookup(name);
        if (!ptr)
            throw ServiceException("Service not found: " + name);
        return *ptr; 
    }

    static Ptr_t create(
        jsonx::json meta, 
        SharedObject::Ptr_t so, 
        const send_t* endpoint);

    static const Service& create_service(
        jsonx::json meta, 
        SharedObject::Ptr_t so, 
        const send_t* endpoint);

    static bool remove_service(const std::string &name) {
        return service_map.erase(name);
    }

    std::string get_name() const { return meta["name"]; }
    const send_t* get_endpoint() const { return &endpoint; }

private:
    static Map_t service_map;

    jsonx::json         meta;
    SharedObject::Ptr_t so;
    send_t              endpoint;
};



#endif // _SERVICE_HPP //
