#ifndef SERVER_HPP
#define SERVER_HPP

#include "session.hpp"

#include <bk/module.h>

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
    std::string get_welcome() const { return meta["welcome"]; }
    bk_error_t start(const lookup_t* lookup_ifc);
    bk_error_t stop();
    void close(Session* session);

private:
    std::unique_ptr<std::thread> worker{nullptr};
    void                         run();

    jsonx::json                  meta;
    lookup_t                     lookup_ifc{};
    service_reg_t                target_service_reg{};
    std::vector<Session::Ptr_t>  sessions{};
    int                          session_id{0};
};
#endif // SERVER_HPP
