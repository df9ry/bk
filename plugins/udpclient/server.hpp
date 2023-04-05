#ifndef _UDPCLIENT_SERVER_HPP
#define _UDPCLIENT_SERVER_HPP

#include <bk/module.h>
#include <bkbase/bkobject.hpp>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>
#include <thread>
#include <atomic>
#include <cassert>

namespace UdpClient {

class ServerException: public std::runtime_error
{
public:
    ServerException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Server: public BkBase::BkObject {
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

    virtual std::string name() const { return meta["name"]; }
    std::string get_welcome() const { return meta["welcome"]; }
    bk_error_t start(const lookup_t* lookup_ifc);
    bk_error_t stop();

    bk_error_t open_session(const char* meta, session_reg_t* reg);
    bk_error_t close_session(const void* session_ctx);
    bk_error_t get(const char* head, resp_f fun, void* ctx);
    bk_error_t post(const char* head, const uint8_t* p_body, size_t c_body);

    bool session_connected{false};
    resp_f response_fun{nullptr};
    void *response_ctx{nullptr};

private:
    void run();

    jsonx::json                  meta;

    std::unique_ptr<std::thread> worker{nullptr};
    std::atomic_bool             quit{false};
    std::atomic_int              sockFD{-1};
    lookup_t                     lookup_ifc{};
    service_t                    target_service_ifc{};
    session_reg_t                target_session_reg{};
    Crc                          crc_type{UNDEF};
};

} // end namespace UdpClient //

#endif // _UDPCLIENT_SERVER_HPP
