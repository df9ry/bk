#ifndef PORT_HPP
#define PORT_HPP

#include <jsonx.hpp>

#include <memory>

class Session;

class Port
{
public:
    typedef std::shared_ptr<Port> Ptr_t;

    static Ptr_t create(Session& session, int id, const jsonx::json &meta);

    ~Port();

    Port() = delete;
    Port(const Port& other) = delete;
    Port(Port&& other) = delete;

    void receive(const jsonx::json &meta, const char *pb, size_t cb);

    const int   id;
    Session&    session;
    jsonx::json meta;

private:
    Port(Session& session, int id, const jsonx::json& meta);
};

#endif // PORT_HPP
