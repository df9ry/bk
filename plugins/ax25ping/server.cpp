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
}
