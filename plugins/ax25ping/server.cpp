#include "server.hpp"
#include "plugin.hpp"

#include <bkbase/bkerror.hpp>

#include <cstring>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace jsonx;

namespace AX25Ping {

Server::Map_t Server::container;

void Server::response_f(void* client_ctx, const char* head, const uint8_t* p_body, size_t c_body)
{
    assert(client_ctx);
    static_cast<Server*>(client_ctx)->response(head, p_body, c_body);
}

Server::Server(const json &_meta):
    BkBase::BkObject(),
    meta{_meta}
{}

Server::~Server()
{
}

Server::Ptr_t Server::create(json meta)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Server(meta)).first->second;
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
    uint8_t body[] { "The quick brown fox" };
    Plugin::dump("Request", body, sizeof(body));
    auto erc = target_session_reg.ifc.post(target_session_reg.ctx, "", body, sizeof(body));
    if (erc != BK_ERC_OK)
        Plugin::error((string("server_ifc->post failed with erc=")
                       + to_string(erc) + ": " + BkBase::bk_error_message(erc)).c_str());
}

void Server::response(const char* head, const uint8_t* p_body, size_t c_body)
{
    Plugin::dump("Response", p_body, c_body);
}

} // end namespace AX25Ping //
