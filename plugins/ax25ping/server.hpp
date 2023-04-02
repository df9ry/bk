#ifndef SERVER_HPP
#define SERVER_HPP

#include "timer.hpp"

#include <bk/module.h>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>

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
    bk_error_t start(const lookup_t* lookup_ifc);
    bk_error_t stop();

private:
    static void response_f(void* client_ctx,
                           const char* head, const char* p_body, size_t c_body);
    void tick();
    void response(const char* head, const char* p_body, size_t c_body);

    jsonx::json                  meta;
    Timer                        timer;
    lookup_t                     lookup_ifc{};
    service_t                    target_service_ifc{};
    int                          session_id{0};
    void                        *server_ctx{nullptr};
    const session_t             *server_ifc{nullptr};
};
#endif // SERVER_HPP
