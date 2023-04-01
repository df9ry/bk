#include "server.hpp"
#include "plugin.hpp"

#include <cstring>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace jsonx;

Server::Map_t Server::container;

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
            Plugin::fatal("lookup_ifc.find_service is null");
        assert(lookup_ifc.find_service);
        auto _target_service_ifc = lookup_ifc.find_service(target.c_str());
        if (!_target_service_ifc)
            Plugin::fatal("Target not found: \"" + target + "\"");
        assert(_target_service_ifc);
        target_service_ifc = *_target_service_ifc;
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

void Server::close(Session* session)
{
    auto iter = find_if(sessions.begin(), sessions.end(),
                    [session](const auto &sp)->bool { return (session == sp.get()); });
    if (iter != sessions.end())
        *iter = nullptr;
    timer.Stop();
}

void Server::tick()
{
    Plugin::debug("Tick");
}
