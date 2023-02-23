#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <unistd.h>
#include <sys/socket.h>

using namespace std;

Session::Session(Server& _server, int _fD, int _id):
    server{_server}, fD{_fD}, id{_id}
{
}

Session::~Session()
{
    if (fD != -1) {
        Plugin::info("Close telnet session \"" + name() + "\"");
        ::close(fD);
    }
}

Session::Ptr_t Session::create(Server& server, int fD, int id)
{
    return Ptr_t(new Session(server, fD, id));
}

string Session::name() const
{
    return server.get_name() + "/" + to_string(id);
}

bk_error_t Session::open()
{
    Plugin::info("Open telnet session \"" + name() + "\"");
    const std::string response = "Hello World\n";
    // send call sends the data you specify as second param and it's length as 3rd param, also returns how many bytes were actually sent
    reader.reset(new thread([this] () { run(); }));
    reader->detach();
    auto bytes_sent = ::send(fD, response.data(), response.length(), 0);
    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close: " + name());
    server.close(this);
}

void Session::run()
{
    char buffer[256];
    while (true) {
        int n = ::recv(fD, buffer, sizeof(buffer)-1, 0);
        if (n <= 0)
            break;
        buffer[n] = '\0';
        Plugin::debug(buffer);
    } // end while //
    server.close(this);
}
