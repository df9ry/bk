#ifndef SESSION_HPP
#define SESSION_HPP

#include <bk/error.h>
#include <bk/module.h>

#include <jsonx.hpp>

#include <memory>
#include <string>
#include <thread>
#include <atomic>

class Server;
class telnet_t;

class Session
{
public:
    typedef std::shared_ptr<Session> Ptr_t;

    static Ptr_t create(Server& server, int fD, int id);

    ~Session();

    Session() = delete;
    Session(const Session& other) = delete;
    Session(Session&& other) = delete;

    std::string       name() const;

    const int         id;
    Server&           server;
    std::atomic<bool> quit{true};

    bk_error_t        open(const service_reg_t& target_service_reg);
    void              close();

    void              input( const char* pb, const size_t cb);
    void              output(const char* pb, const size_t cb);

private:
    Session(Server& server, int fD, int id);

    const std::atomic<int>       fD;
    telnet_t*                    telnet;
    std::unique_ptr<std::thread> reader{nullptr};
    service_reg_t                target_service_reg{};
    session_t                    target_session_ifc{};
    void*                        target_session_ctx{nullptr};

    void                         run();
};

#endif // SESSION_HPP
