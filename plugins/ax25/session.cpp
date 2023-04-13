#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <jsonx.hpp>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

Session::Session(Server& _server, int _id):
    server{_server}, id{_id}, dlc{*this}
{
}

Session::~Session()
{
    Plugin::info("Close ax25 session \"" + name() + "\"");
}

Session::Ptr_t Session::create(Server& server, int id)
{
    return Ptr_t(new Session(server, id));
}

string Session::name() const
{
    return server.name() + "/" + to_string(id);
}

bk_error_t Session::open()
{
    Plugin::info("Open ax25 session \"" + name() + "\"");
    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close: " + name());
    server.close(this);
}

bk_error_t Session::get(const char* head, resp_f fun, void* ctx)
{
    return BK_ERC_NOT_IMPLEMENTED;
}

bk_error_t Session::post(const char* head, const uint8_t* p_body, size_t c_body)
{
    return BK_ERC_NOT_IMPLEMENTED;
}
