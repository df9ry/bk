#ifndef SESSION_HPP
#define SESSION_HPP

#include <bk/error.h>

#include <memory>
#include <string>
#include <thread>
#include <atomic>

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

    std::string       name() const;

    const int         id;
    Server&           server;
    std::atomic<bool> quit{true};

    bk_error_t        open();
    void              close();

    void              input( const char* pb, const size_t cb);
    void              output(const char* pb, const size_t cb);

private:
    Session(Server& server, int fD, int id);

    const std::atomic<int>       fD;
    std::unique_ptr<std::thread> reader{nullptr};

    void                         run();
};

#endif // SESSION_HPP
