#ifndef SERVER_HPP
#define SERVER_HPP

#include "session.hpp"

#include <bk/session.h>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>
#include <thread>

class ServerException: public std::runtime_error
{
public:
    ServerException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Server {
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

    std::string get_name() const { return meta["name"]; }
    const session_admin_t* get_session_admin() const { return &session_admin_ifc; }
    bk_error_t start();
    bk_error_t stop();
    void close(Session* session);

private:
    std::unique_ptr<std::thread> worker{nullptr};
    void                         run();

    jsonx::json                  meta;
    session_admin_t              session_admin_ifc;
    std::vector<Session::Ptr_t>  sessions{};
    int                          session_id{0};
};
#endif // SERVER_HPP
