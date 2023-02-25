#include "port.hpp"
#include "server.hpp"
#include "plugin.hpp"
#include "utils.hpp"

using namespace std;

Port::Ptr_t Port::create(Server& server, int id, const jsonx::json &meta)
{
    auto port = Ptr_t(new Port(server, id, meta));
    Plugin::info("Creating port \"" + port->name() + "\"");
    return port;
}

void Port::receive(const jsonx::json &meta, const char *pb, size_t cb)
{
    switch (string_2_kind(meta["kind"])) {
    default:
        break;
    } // end switch //
}

Port::~Port()
{}

Port::Port(Server& _server, int _id, const jsonx::json &_meta):
    server{_server}, id{_id}, meta{_meta}
{}

string Port::name() const
{
    return server.get_name() + "/" + to_string(id);
}
