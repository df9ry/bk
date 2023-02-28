#include "port.hpp"
#include "server.hpp"
#include "session.hpp"
#include "plugin.hpp"
#include "utils.hpp"

using namespace std;

Port::Ptr_t Port::create(Server& server, int id, const jsonx::json &meta)
{
    auto port = Ptr_t(new Port(server, id, meta));
    Plugin::info("Creating port \"" + port->name() + "\"");
    return port;
}

void Port::receive(Session& session, const jsonx::json &meta, const char *pb, size_t cb)
{
    switch (string_2_kind(meta["kind"])) {
    case TX_QUEUE_S:
        tx_queue_size(session, meta);
        break;
    case CONNECT:
        connect(session, meta);
        break;
    case DISCONNECT:
        disconnect(session, meta);
        break;
    default:
        Plugin::debug("Unrecognized frame: " + meta_2_string(meta));
        break;
    } // end switch //
}

void Port::tx_queue_size(Session& session, const jsonx::json& meta)
{
    string call_from = meta["from"];
    string call_to   = meta["to"];
    //Plugin::debug("Agw: Asking TX queue size for " +
    //                call_to + " de " + call_from +
    //                " on port " + name());

    struct __attribute__((__packed__)) port_info_reply_t {
        agw_header_t header;
        uint32_t     n;
    };
    union __attribute__((__packed__)) {
        port_info_reply_t structured;
        char flat[sizeof(port_info_reply_t)];
    } frame;
    ::memset(frame.flat, 0x00, sizeof(frame.flat));
    frame.structured.header.kind = TX_QUEUE_S;
    frame.structured.header.port = id;
    frame.structured.header.data_length = sizeof(uint32_t);
    buf10_set(frame.structured.header.call_from, call_from);
    buf10_set(frame.structured.header.call_to,   call_to);
    frame.structured.n = 0;
    //Plugin::dump("TX", frame.flat, sizeof(frame.flat));
    session.transmit(frame.flat, sizeof(frame.flat));
}

void Port::connect(Session& session, const jsonx::json& meta)
{
    string call_from = meta["from"];
    string call_to   = meta["to"];
    Plugin::debug("Agw: Request connect " +
                    call_to + " de " + call_from +
                    " on port " + name());
    connected(session, call_to, call_from, false);
}

void Port::disconnect(Session& session, const jsonx::json& meta)
{
    string call_from = meta["from"];
    string call_to   = meta["to"];
    Plugin::debug("Agw: Request disconnect " +
                    call_to + " de " + call_from +
                    " on port " + name());
    disconnected(session, call_from, call_to, false);
}

void Port::connected(Session& session, const std::string& local_call,
                  const std::string& peer_call, bool initiated)
{
    Plugin::debug("Agw: Connect on port " + name());
    string info;
    {
        stringstream ss;
        ss << "*** CONNECTED " << (initiated ? "With " : "To Station ") << peer_call;
        info = ss.str();
    }
    vector<char> frame;
    {
        agw_header_t header;
        ::memset(&header, 0x00, sizeof(header));
        header.kind = CONNECT;
        header.port = id;
        header.data_length = sizeof(uint32_t);
        buf10_set(header.call_from, local_call);
        buf10_set(header.call_to,   peer_call);
        header.data_length = info.length();
        char *p_header = (char*)&header;
        frame.insert(frame.end(), p_header, p_header + sizeof(header));
    }
    frame.insert(frame.end(), info.begin(), info.end());
    session.transmit(frame.data(), frame.size());
}

void Port::disconnected(Session& session, const std::string& local_call,
                  const std::string& peer_call, bool timeout)
{
    Plugin::debug("Agw: Disconnect on port " + name());
    string info;
    {
        stringstream ss;
        ss << "*** DISCONNECTED " << (timeout ? "RETRYOUT With " : "From Station ") << peer_call;
        info = ss.str();
    }
    vector<char> frame;
    {
        agw_header_t header;
        ::memset(&header, 0x00, sizeof(header));
        header.kind = DISCONNECT;
        header.port = id;
        header.data_length = sizeof(uint32_t);
        buf10_set(header.call_from, local_call);
        buf10_set(header.call_to,   peer_call);
        header.data_length = info.length();
        char *p_header = (char*)&header;
        frame.insert(frame.end(), p_header, p_header + sizeof(header));
    }
    frame.insert(frame.end(), info.begin(), info.end());
    session.transmit(frame.data(), frame.size());
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
