#ifndef SESSION_HPP
#define SESSION_HPP

#include <bk/error.h>

#include <memory>
#include <string>
#include <thread>

class Server;

class Session
{
public:
    typedef std::shared_ptr<Session> Ptr_t;

    static Ptr_t create(Server& server, int fD, int id);

    ~Session();

    Session() = delete;
    Session(const Session& other) = delete;
    Session(Session&& other) = delete;

    std::string name() const;

    const int     id;
    Server&       server;

    bk_error_t    open();
    void          close();

private:
    Session(Server& server, int fD, int id);

    std::unique_ptr<std::thread> reader{nullptr};
    void                         run();
    const int                    fD;
};

#endif // SESSION_HPP
