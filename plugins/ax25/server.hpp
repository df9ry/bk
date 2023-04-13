#ifndef SERVER_HPP
#define SERVER_HPP

#include "session.hpp"

#include <bk/module.h>
#include <bkbase/bkobject.hpp>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>

class ServerException: public std::runtime_error
{
public:
    ServerException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Server: public BkBase::BkObject {
public:
    typedef std::shared_ptr<Server> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    static Map_t container;

    static bool is_defined(const std::string &name) {
        return container.contains(name);
    }

    static Ptr_t lookup(const std::string &name) {
        auto iter = container.find(name);
        return (iter != container.end()) ? iter->second : nullptr;
    }

    static const Server& lookup_service(const std::string &name) {
        auto ptr = lookup(name);
        if (!ptr)
            throw ServerException("Server not found: " + name);
        return *ptr;
    }

    static Ptr_t create(jsonx::json meta);

    static bool remove(const std::string &name) {
        return container.erase(name);
    }

    Server(const jsonx::json &meta);
    ~Server();

    Server() = delete;
    Server(const Server& other) = delete;
    Server(Server&& other) = delete;

    std::string name() const { return meta["name"]; }
    bk_error_t start(const lookup_t* lookup_ifc);
    bk_error_t stop();
    bk_error_t open_session(const char* meta, session_reg_t* reg);
    bk_error_t close_session(const void* session_ctx);
    void close(Session* session);

private:
    jsonx::json                  meta;
    lookup_t                     lookup_ifc{};
    service_t                    target_service_ifc{};
    std::vector<Session::Ptr_t>  sessions{};
    int                          session_id{0};
};
#endif // SERVER_HPP
