#ifndef SESSION_HPP
#define SESSION_HPP

#include <bk/error.h>
#include <bk/module.h>

#include <jsonx.hpp>

#include <memory>
#include <string>

class Server;

class Session
{
public:
    typedef std::shared_ptr<Session> Ptr_t;

    static Ptr_t create(Server& server, int id);

    ~Session();

    Session() = delete;
    Session(const Session& other) = delete;
    Session(Session&& other) = delete;

    std::string       name() const;

    const int         id;
    jsonx::json       meta;
    Server&           server;

    bk_error_t        open();
    void              close();

private:
    Session(Server& server, int id);

    service_t          target_service_ifc{};
    session_t          target_session_ifc{};
    void*              target_session_ctx{nullptr};
};

#endif // SESSION_HPP
