#include "server.hpp"
#include "plugin.hpp"

#include <bkbase/bkerror.hpp>
#include <ax25base/ax25_u.hpp>

#include <cstring>
#include <algorithm>
#include <cassert>

#define MONITOR

using namespace std;
using namespace jsonx;
using namespace AX25Base;

namespace AX25Ping {

Server::Map_t Server::container;

void Server::response_f(void* client_ctx, const char* head,
                        const uint8_t* p_body, size_t c_body)
{
    assert(client_ctx);
    static_cast<Server*>(client_ctx)->response(head, p_body, c_body);
}

Server::Server(const json &_meta):
    BkBase::BkObject(),
    meta{_meta}
{
}

Server::~Server()
{
}

Server::Ptr_t Server::create(json meta)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    auto server = container.emplace(name, new Server(meta)).first->second;
    // Create frame:
    string src = meta["src"];
    auto src_call = src.empty() ? L2Callsign::CQ : L2Callsign(src);
    string dst = meta["dst"];
    auto dst_call = dst.empty() ? L2Callsign::CQ : L2Callsign(dst);
    auto header = AX25Header::Ptr(new AX25Header(src_call, dst_call));
    auto payload = AX25Payload::Ptr(new AX25_TEST(true));
    server->frame.reset(new AX25Frame(header, payload));
    return server;
}

bk_error_t Server::start(const lookup_t* _lookup_ifc)
{
    Plugin::info("Start server \"" + name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;

    string target = meta["target"];
    if (!target.empty()) {
        if (!lookup_ifc.find_service)
            Plugin::fatal("AX25 Ping: lookup_ifc.find_service is null");
        assert(lookup_ifc.find_service);
        auto _target_service_reg = lookup_ifc.find_service(target.c_str());
        if (!_target_service_reg)
            Plugin::fatal("AX25 Ping: Target not found: \"" + target + "\"");
        assert(_target_service_reg);
        target_service_reg = *_target_service_reg;
        // Connect to target server:
        if (!target_service_reg.ifc.open_session)
            Plugin::fatal("AX25 Ping: target_service_ifc.open_session is null");
        assert(target_service_reg.ifc.open_session);
        if (!target_service_reg.ifc.close_session)
            Plugin::fatal("AX25 Ping: target_service_ifc.close_session is null");
        assert(target_service_reg.ifc.close_session);
        // Create target session:
        auto erc = target_service_reg.ifc.open_session(
            target_service_reg.ctx, "{}", &target_session_reg);
        if (erc != BK_ERC_OK)
            Plugin::fatal((string("AX25 Ping: open_session() failed with erc=")
                            + to_string(erc) + ": "
                           + BkBase::bk_error_message(erc)).c_str());
        if (!target_session_reg.ifc.get)
            Plugin::fatal("AX25 Ping: get() is not defined");
        assert(target_session_reg.ifc.get);
        if (!target_session_reg.ifc.post)
            Plugin::fatal("AX25 Ping: post() is not defined");
        assert(target_session_reg.ifc.post);
        // Set callback function:
        erc = target_session_reg.ifc.get(target_session_reg.ctx, "", response_f, this);
        if (erc != BK_ERC_OK)
            Plugin::fatal((string("AX25 Ping: server_ifc->get failed with erc=")
                           + to_string(erc) + ": " + BkBase::bk_error_message(erc)).c_str());

        // Start timer:
        int interval = meta["interval"];
        if (!interval)
            interval = 1000;
        chrono::milliseconds d(interval);
        timer.Start(d, [this] () { tick(); }, false);
    }
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop server \"" + name() + "\"");
    return BK_ERC_OK;
}

void Server::tick()
{
    Plugin::debug("AX25Ping tick");
    assert(target_session_reg.ifc.post);
    const auto& octets = frame->GetOctets();
    auto pb = octets->data();
    auto cb = octets->size();
    Plugin::dump("Request", pb, cb);
#ifdef MONITOR
    {
        auto p2 = static_cast<uint8_t*>(pb + cb);
        auto data(OctetArray(new AX25Base::octet_vector_t(pb, p2)));
        auto frame = AX25Frame(data);
        Plugin::info(frame.ToString());
    }
#endif
    auto erc = target_session_reg.ifc.post(target_session_reg.ctx, "", pb, cb);
    if (erc != BK_ERC_OK)
        Plugin::error((string("server_ifc->post failed with erc=")
                       + to_string(erc) + ": " + BkBase::bk_error_message(erc)).c_str());
}

void Server::response(const char* head, const uint8_t* p_body, size_t c_body)
{
    Plugin::dump("Response", p_body, c_body);
}

} // end namespace AX25Ping //
