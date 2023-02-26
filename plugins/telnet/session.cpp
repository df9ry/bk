#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <libtelnet.h>
#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

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

bk_error_t Session::open(const service_t& _target_service_ifc)
{
    Plugin::info("Open telnet session \"" + name() + "\"");
    target_service_ifc = _target_service_ifc;
    // Get new session interface:
    session_t *_target_session_ifc;
    auto erc = _target_service_ifc.open_session("{}",
                                                &_target_session_ifc,
                                                &target_session_id);
    if (erc)
        return erc;
    target_session_ifc = *_target_session_ifc;

    reader.reset(new thread([this] () { run(); }));
    reader->detach();

    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close: " + name());
    if (target_service_ifc.close_session)
        target_service_ifc.close_session(target_session_id);
    target_session_id = 0;
    server.close(this);
}

void Session::run()
{
    string prompt = server.get_prompt();
    if (!prompt.empty())
        output(prompt.c_str(), prompt.length());
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
    if (target_session_ifc.xch) {
        target_session_ifc.xch(target_session_id, "{}", pb, cb,
                               [] (int session_id,
                                   const char* response_meta,
                                   const char* p_response_body,
                                   size_t      c_response_body,
                                   void*       user_data)
        {
            auto session = static_cast<Session*>(user_data);
            session->output(p_response_body, c_response_body);
        }, this);
    } else {
        Plugin::dump("Missed put", pb, cb);
    }
}
