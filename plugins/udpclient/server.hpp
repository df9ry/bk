#ifndef SERVER_HPP
#define SERVER_HPP

#include <bk/module.h>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>
#include <thread>
#include <atomic>
#include <cassert>

struct addrinfo;

class ServerException: public std::runtime_error
{
public:
    ServerException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Server {
public:
    typedef std::shared_ptr<Server> Ptr_t;
    typedef std::map<std::string, Ptr_t> Map_t;

    enum Crc { UNDEF, NONE, CRC_B };

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

private:
    static inline Server *self(void* server_ctx) {
        assert(server_ctx);
        return static_cast<Server*>(server_ctx);
    }

    void run();

    bk_error_t open_session(void** server_ctx_ptr, const char* meta, session_t** ifc_ptr);
    bk_error_t close_session();
    bool session_connected{false};
    resp_f response_f{nullptr};
    service_t my_service_ifc {
        .open_session = [] (void* client_loc_ctx, void** server_ctx_ptr,
                           const char* meta, session_t** ifc_ptr) -> bk_error_t
        {
            return self(client_loc_ctx)->open_session(server_ctx_ptr, meta, ifc_ptr);
        },
        .close_session = [] (void* server_ctx) -> bk_error_t
        {
            return self(server_ctx)->close_session();
        }
    };

    bk_error_t get(const char* head, resp_f fun);
    bk_error_t post(const char* head, const char* p_body, size_t c_body);
    session_t my_session_ifc {
        .get = [] (void* server_ctx, const char* head, resp_f fun) -> bk_error_t
        {
            return self(server_ctx)->get(head, fun);
        },
        .post = [] (void* server_ctx, const char* head, const char* p_body, size_t c_body) -> bk_error_t
        {
            return self(server_ctx)->post(head, p_body, c_body);
        }
    };

    jsonx::json                  meta;

    std::unique_ptr<std::thread> worker{nullptr};
    addrinfo                    *info{nullptr};
    void                        *target_addr{nullptr};
    std::atomic_bool             quit{false};
    std::atomic_int              sockFD{-1};
    lookup_t                     lookup_ifc{};
    service_t                    target_service_ifc{};
    Crc                          crc_type{UNDEF};
};

#endif // SERVER_HPP
