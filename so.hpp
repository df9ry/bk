#ifndef _SO_HPP
#define _SO_HPP

#include <string>
#include <memory>
#include <map>
#include <exception>
#include <filesystem>

#include <jsonx.hpp>

#include "bk/error.h"

class SharedObjectException: public std::runtime_error
{
public:
    SharedObjectException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class SharedObject {
public:
    typedef std::shared_ptr<SharedObject> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    static Map_t container;

    static bool is_loaded(const std::string &id) {
        return container.contains(id);
    }

    static Ptr_t lookup(const std::string &id) {
        auto iter = container.find(id);
        return (iter != container.end()) ? iter->second : nullptr;
    }

    static const SharedObject& lookup_so(const std::string &id) {
        auto ptr = lookup(id);
        if (!ptr)
            throw SharedObjectException("Shared object not found: " + id);
        return *ptr; 
    }

    static Ptr_t create(std::filesystem::path, jsonx::json meta);

    static const SharedObject& create_so(std::filesystem::path, jsonx::json meta);

    static bool drop(const std::string &id) {
        return container.erase(id);
    }
    
    const std::string id;
    std::string       get_name() const { return meta["name"]; }

    SharedObject(jsonx::json meta);
    ~SharedObject();

    SharedObject() = delete;
    SharedObject(const SharedObject& other) = delete;
    SharedObject(SharedObject&& other) = delete;

    bool load(const std::string &path);
    void* getsym(const std::string &name);
    bk_error_t start();
    bk_error_t stop();

    std::string error_text() const { return error; }

private:
    jsonx::json       meta;
    void*             handle{nullptr};
    std::string       error{};

    void unload();
};

#endif // _SO_HPP //
