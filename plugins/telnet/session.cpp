#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <unistd.h>
#include <sys/socket.h>

using namespace std;

int Session::lastID{0};

Session::Session(const Server& _server, int _fD):
    server{_server}, fD{_fD}, id{++lastID}
{
    Plugin::info("Open telnet session \"" + name() + "\"");
    const std::string response = "Hello World\n";
    // send call sends the data you specify as second param and it's length as 3rd param, also returns how many bytes were actually sent
    auto bytes_sent = send(fD, response.data(), response.length(), 0);
}

Session::~Session()
{
    Plugin::info("Close telnet session \"" + name() + "\"");
    close(fD);
}

Session::Ptr_t Session::create(const Server& server, int fD)
{
    return Ptr_t(new Session(server, fD));
}

string Session::name() const
{
    return server.get_name() + "/" + to_string(id);
}
