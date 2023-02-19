#ifndef _SO_HPP
#define _SO_HPP

#include <string>
#include <memory>
#include <map>
#include <exception>
#include <filesystem>

#include <jsonx.hpp>

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
    jsonx::json       meta;

    SharedObject(jsonx::json meta);
    ~SharedObject();

    SharedObject() = delete;
    SharedObject(const SharedObject& other) = delete;
    SharedObject(SharedObject&& other) = delete;

    bool load(const std::string &path);
    void* getsym(const std::string &name);

    std::string error_text() const { return error; }

private:
    void*             handle{nullptr};
    std::string       error{};

    void unload();
};

#endif // _SO_HPP //