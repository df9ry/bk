#include "server.hpp"
#include "plugin.hpp"

#include <cstring>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace jsonx;

static const session_t my_session_ifc {
    .get = [] (void* session_ctx, const char* head, resp_f fun, void* ctx) -> bk_error_t
    {
        try {
            return BkBase::self<Session>(session_ctx).get(head, fun, ctx);
        }
        catch (const exception& ex) {
            Plugin::error(string("AX25::get() exception: ") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    },
    .post = [] (void* session_ctx, const char* head, const uint8_t* p_body, size_t c_body) -> bk_error_t
    {
        try {
            return BkBase::self<Session>(session_ctx).post(head, p_body, c_body);
        }
        catch (const exception& ex) {
            Plugin::error(string("AX25::post() exception: ") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    }
};

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
    Plugin::info("Start server \"" + name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop server \"" + name() + "\"");
    return BK_ERC_OK;
}

void Server::close(Session* session)
{
    auto iter = find_if(sessions.begin(), sessions.end(),
                    [session](const auto &sp)->bool { return (session == sp.get()); });
    if (iter != sessions.end())
        *iter = nullptr;
}

bk_error_t Server::open_session(const char* meta, session_reg_t* reg)
{
    return BK_ERC_NOT_IMPLEMENTED;
}

bk_error_t Server::close_session(const void* session_ctx)
{
    return BK_ERC_NOT_IMPLEMENTED;
}
