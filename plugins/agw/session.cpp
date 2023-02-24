#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>

using namespace std;

Session::Session(Server& _server, int _fD, int _id):
    server{_server}, fD{_fD}, id{_id}
{
}

Session::~Session()
{
    if (fD != -1) {
        Plugin::info("Close agw session \"" + name() + "\"");
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
    Plugin::info("Open agw session \"" + name() + "\"");
    reader.reset(new thread([this] () { run(); }));
    reader->detach();
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
    quit = false;
    while (!quit) {
        int n = ::recv(fD, buffer, sizeof(buffer), 0);
        if (n <= 0)
            break;
        Plugin::dump("RX", buffer, n);
    } // end while //
    server.close(this);
}

void Session::output(const char* pb, const size_t cb)
{
    auto cbSent = ::send(fD, pb, cb, 0);
    if (cbSent != cb) {
        Plugin::error("AGW " + name() + ": Error sending data");
        quit = true;
    }
}

void Session::input(const char* pb, const size_t cb)
{
    char buffer[cb+1];
    memcpy(buffer, pb, cb);
    for (size_t i = 0; i < cb; ++i)
        if (!isprint(buffer[i]))
            buffer[i] = '~';
    buffer[cb] = '\0';
    Plugin::debug("AGW " + name() + " receive: \"" + buffer + "\"");
}
