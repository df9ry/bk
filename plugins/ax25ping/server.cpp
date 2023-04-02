#include "server.hpp"
#include "plugin.hpp"

#include <cstring>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace jsonx;

Server::Map_t Server::container;

void Server::response_f(void* client_ctx, const char* head, const char* p_body, size_t c_body)
{
    assert(client_ctx);
    static_cast<Server*>(client_ctx)->response(head, p_body, c_body);
}

Server::Server(const json &_meta):
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
    Plugin::info("Start server \"" + get_name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;

    string target = meta["target"];
    if (!target.empty()) {
        if (!lookup_ifc.find_service)
            Plugin::fatal("AX25 Ping: lookup_ifc.find_service is null");
        assert(lookup_ifc.find_service);
        auto _target_service_ifc = lookup_ifc.find_service(target.c_str());
        if (!_target_service_ifc)
            Plugin::fatal("AX25 Ping: Target not found: \"" + target + "\"");
        assert(_target_service_ifc);
        target_service_ifc = *_target_service_ifc;
        // Connect to target:
        if (!target_service_ifc.open_session)
            Plugin::fatal("AX25 Ping: target_service_ifc.open_session is null");
        assert(target_service_ifc.open_session);
        auto erc = target_service_ifc.open_session(this, &server_ctx, "", &server_ifc);
        if (erc != BK_ERC_OK)
            Plugin::fatal("AX25 Ping: target_service_ifc.open_session failed with erc=" + to_string(erc));
        if (!server_ifc)
            Plugin::fatal("AX25 Ping: server_ifc is null");
        assert(server_ifc);
        // Set callback function:
        if (!server_ifc->get)
            Plugin::fatal("AX25 Ping: server_ifc.get is null");
        assert(server_ifc->get);
        erc = server_ifc->get(server_ctx, "", response_f);
        if (erc != BK_ERC_OK)
            Plugin::fatal("AX25 Ping: server_ifc->get failed with erc=" + to_string(erc));
        if (!server_ifc->post)
            Plugin::fatal("AX25 Ping: server_ifc.post is null");
        assert(server_ifc->post);
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
    Plugin::info("Stop server \"" + get_name() + "\"");
    return BK_ERC_OK;
}

void Server::tick()
{
    Plugin::debug("AX25Ping tick");
    assert(server_ifc->post);
    char body[] { 0x00, 0x00 };
    auto erc = server_ifc->post(server_ctx, "", body, sizeof(body));
    if (erc != BK_ERC_OK)
        Plugin::error("server_ifc->post failed with erc=" + to_string(erc));
}

void Server::response(const char* head, const char* p_body, size_t c_body)
{
    Plugin::dump("Response", p_body, c_body);
}
