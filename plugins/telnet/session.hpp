#ifndef SESSION_HPP
#define SESSION_HPP

#include <memory>
#include <string>

class Server;

class Session
{
public:
    typedef std::shared_ptr<Session> Ptr_t;

    static Ptr_t create(const Server& server, int fD);

    ~Session();

    Session() = delete;
    Session(const Session& other) = delete;
    Session(Session&& other) = delete;

    std::string name() const;

    const int     id;
    const Server& server;

private:
    static int    lastID;

    Session(const Server& server, int fD);

     const int     fD;
};

#endif // SESSION_HPP
