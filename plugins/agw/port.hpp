#ifndef PORT_HPP
#define PORT_HPP

#include <jsonx.hpp>

#include <memory>

class Server;

class Port
{
public:
    typedef std::shared_ptr<Port> Ptr_t;

    static Ptr_t create(Server& server, int id, const jsonx::json &meta);

    ~Port();

    Port() = delete;
    Port(const Port& other) = delete;
    Port(Port&& other) = delete;

    void receive(const jsonx::json &meta, const char *pb, size_t cb);
    std::string name() const;

    const int   id;
    Server&     server;
    jsonx::json meta;

private:
    Port(Server& server, int id, const jsonx::json& meta);
};

#endif // PORT_HPP
