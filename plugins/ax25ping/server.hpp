#ifndef _AX25PING_SERVER_HPP
#define _AX25PING_SERVER_HPP

#include "timer.hpp"

#include <bk/module.h>
#include <bkbase/bkobject.hpp>
#include <ax25base/ax25frame.hpp>

#include <jsonx.hpp>

#include <stdexcept>
#include <memory>
#include <map>

namespace AX25Ping {

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

private:
    static void response_f(void* client_ctx,
                           const char* head, const uint8_t* p_body, size_t c_body);
    void tick();
    void response(const char* head, const uint8_t* p_body, size_t c_body);

    jsonx::json                  meta;
    Timer                        timer;
    lookup_t                     lookup_ifc{};
    service_reg_t                target_service_reg{};
    session_reg_t                target_session_reg{};
    AX25Base::AX25Frame::Ptr     frame{nullptr};
};

} // end namespace AX25Ping //

#endif // _AX25PING_SERVER_HPP
