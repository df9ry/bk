#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <jsonx.hpp>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

Session::Session(Server& _server, int _id):
    server{_server}, id{_id}
{
}

Session::~Session()
{
    Plugin::info("Close UDP client session \"" + name() + "\"");
}

Session::Ptr_t Session::create(Server& server, int id)
{
    return Ptr_t(new Session(server, id));
}

string Session::name() const
{
    return server.get_name() + "/" + to_string(id);
}

bk_error_t Session::open()
{
    Plugin::info("Open UDP client session \"" + name() + "\"");
    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close: " + name());
    server.close(this);
}
