#ifndef PORT_HPP
#define PORT_HPP

#include <jsonx.hpp>

#include <memory>

class Server;
class Session;

class Port
{
public:
    typedef std::shared_ptr<Port> Ptr_t;

    static Ptr_t create(Server& server, int id, const jsonx::json &meta);

    ~Port();

    Port() = delete;
    Port(const Port& other) = delete;
    Port(Port&& other) = delete;

    void receive(Session& session, const jsonx::json &meta, const char *pb, size_t cb);
    std::string name() const;

    const int   id;
    Server&     server;
    jsonx::json meta;

private:
    Port(Server& server, int id, const jsonx::json& meta);

    void connect(Session& session, const jsonx::json& meta);
    void disconnect(Session& session, const jsonx::json& meta);
    void tx_queue_size(Session& session, const jsonx::json& meta);
    void connected(Session& session, const std::string& local_call,
                   const std::string& peer_call, bool initiated);
    void disconnected(Session& session, const std::string& local_call,
                      const std::string& peer_call, bool timeout);
};

#endif // PORT_HPP
