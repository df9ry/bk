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

Session::Ptr_t Server::find(const string& name)
{
    auto iter = sessions.find(name);
    return (iter != sessions.end()) ? iter->second : Session::Ptr_t(nullptr);
}

void Server::close(const std::string& name)
{
    sessions.erase(name);
}

bk_error_t Server::open_session(const char* meta, session_reg_t* reg)
{
    if (!reg) {
        Plugin::warning("Attempt to open session without registration");
        return BK_ERC_NO_SESSION_IFC_PTR;
    }
    assert(reg);
    reg->ifc = my_session_ifc;
    json obj;
    string session_name;
    try {
        obj.parse(meta);
        session_name = obj["name"].toString();
    }
    catch (const exception& ex) {
        Plugin::warning("Invalid meta: \"" + string(meta) + "\"");
        return BK_ERC_INV_META;
    }
    if (sessions.contains(session_name)) {
        Plugin::warning("Session already registered: \"" + session_name + "\"");
        return BK_ERC_ENGAGED;
    }
    auto session = Session::create(*this, meta);
    sessions.emplace(session_name, session);
    reg->ctx = session.get();
    return BK_ERC_OK;
}

bk_error_t Server::close_session(const void* session_ctx)
{
    return BK_ERC_NOT_IMPLEMENTED;
}
