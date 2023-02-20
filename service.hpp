#ifndef _SERVICE_HPP
#define _SERVICE_HPP

#include "so.hpp"
#include "bk/send.h"

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

class Plugin {
public:
    typedef std::shared_ptr<Plugin> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    Plugin(
        const jsonx::json &meta, 
        SharedObject::Ptr_t so,
        const send_t* endpoint);
    ~Plugin();

    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = delete;

    static bool is_defined(const std::string &name) {
        return container.contains(name);
    }

    static Ptr_t lookup(const std::string &name) {
        auto iter = container.find(name);
        return (iter != container.end()) ? iter->second : nullptr;
    }

    static const Plugin& lookup_service(const std::string &name) {
        auto ptr = lookup(name);
        if (!ptr)
            throw ServiceException("Service not found: " + name);
        return *ptr; 
    }

    static Ptr_t create(
        jsonx::json meta, 
        SharedObject::Ptr_t so, 
        const send_t* endpoint);

    static const Plugin& create_service(
        jsonx::json meta, 
        SharedObject::Ptr_t so, 
        const send_t* endpoint);

    static bool remove_service(const std::string &name) {
        return container.erase(name);
    }

    std::string get_name() const { return meta["name"]; }
    const send_t* get_endpoint() const { return &endpoint; }

private:
    static Map_t container;

    jsonx::json         meta;
    SharedObject::Ptr_t so;
    send_t              endpoint;
};

#endif // _SERVICE_HPP //
