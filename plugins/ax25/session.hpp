#ifndef SESSION_HPP
#define SESSION_HPP

#include "dlc.hpp"

#include <bk/error.h>
#include <bk/module.h>
#include <bkbase/bkobject.hpp>

#include <jsonx.hpp>

#include <memory>
#include <string>

class Server;

class Session: public BkBase::BkObject
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
    bk_error_t        get(const char* head, resp_f fun, void* ctx);
    bk_error_t        post(const char* head, const uint8_t* p_body, size_t c_body);

    ax25::DLC         dlc;

private:
    Session(Server& server, int id);

    service_t         target_service_ifc{};
    session_t         target_session_ifc{};
    void*             target_session_ctx{nullptr};
};

#endif // SESSION_HPP
