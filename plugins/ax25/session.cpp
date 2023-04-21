#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <jsonx.hpp>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

Session::Session(Server& _server, const jsonx::json& _meta): BkBase::BkObject(),
    server{_server}, meta{_meta}, my_name{meta["name"].toString()}, dlc{*this}
{
}

Session::~Session()
{
    Plugin::info("Close ax25 session \"" + my_name + "\"");
}

Session::Ptr_t Session::create(Server& server, const jsonx::json& meta)
{
    return Ptr_t(new Session(server, meta));
}

string Session::name() const
{
    return server.name() + "/" + my_name;
}

bk_error_t Session::open()
{
    Plugin::info("Open ax25 session \"" + name() + "\"");
    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close ax25 session: \"" + name() + "\"");
    server.close(my_name);
}

bk_error_t Session::get(const char* head, resp_f fun, void* ctx)
{
    client_resp_f = fun;
    client_context = ctx;
    return BK_ERC_OK;
}

bk_error_t Session::post(const char* head, const uint8_t* p_body, size_t c_body)
{
    return BK_ERC_NOT_IMPLEMENTED;
}
