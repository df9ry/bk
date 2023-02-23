#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <libtelnet.h>
#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>

using namespace std;

static const telnet_telopt_t telopts[] = {
    { TELNET_TELOPT_ECHO,      TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_TTYPE,     TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_BINARY,    TELNET_WONT, TELNET_WONT },
    { TELNET_TELOPT_NAWS,      TELNET_WILL, TELNET_DONT },
    { -1, 0, 0 }
};

static void telnet_event_handler(telnet_t       *telnet,
                                 telnet_event_t *ev,
                                 void           *user_data)
{
    auto self = (Session *)user_data;
    assert(self);

    switch (ev->type) {
    case TELNET_EV_DATA:
        self->input(ev->data.buffer, ev->data.size);
        break;
    case TELNET_EV_SEND:
        self->output(ev->data.buffer, ev->data.size);
        break;
    case TELNET_EV_ERROR:
        Plugin::error(ev->error.msg);
        self->quit = true;
        break;
    default:
        break;
    } // end switch //
}

Session::Session(Server& _server, int _fD, int _id):
    server{_server}, fD{_fD}, id{_id}
{
    telnet = telnet_init(telopts, telnet_event_handler,
                         TELNET_FLAG_NVT_EOL, this);
}

Session::~Session()
{
    if (fD != -1) {
        Plugin::info("Close telnet session \"" + name() + "\"");
        ::close(fD);
    }
    telnet_free(telnet);
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
    reader.reset(new thread([this] () { run(); }));
    reader->detach();

    output("Test\n", 5);

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
        telnet_recv(telnet, buffer, n);
    } // end while //
    server.close(this);
}

void Session::output(const char* pb, const size_t cb)
{
    auto cbSent = ::send(fD, pb, cb, 0);
    if (cbSent != cb) {
        Plugin::error("Telnet " + name() + ": Error sending data");
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
    Plugin::debug("Telnet " + name() + " receive: \"" + buffer + "\"");
}
