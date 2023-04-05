#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"

#include <bkbase/bkerror.hpp>

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
        self->input((const uint8_t*)ev->data.buffer, ev->data.size);
        break;
    case TELNET_EV_SEND:
        self->output((const uint8_t*)ev->data.buffer, ev->data.size);
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
    Plugin::info("Close telnet session \"" + name() + "\"");
    ::close(fD);
    telnet_free(telnet);
    if (target_service_reg.ifc.close_session)
        target_service_reg.ifc.close_session(target_service_reg.ctx, target_session_reg.ctx);
}

Session::Ptr_t Session::create(Server& server, int fD, int id)
{
    return Ptr_t(new Session(server, fD, id));
}

string Session::name() const
{
    return server.get_name() + "/" + to_string(id);
}

static void my_resp_fun(void* client_ctx, const char* head, const uint8_t* p_body, size_t c_body)
{
    assert(client_ctx);
    auto session = static_cast<Session*>(client_ctx);
    session->output(p_body, c_body);
}

bk_error_t Session::open(const service_reg_t& _target_service_reg)
{
    Plugin::info("Open telnet session \"" + name() + "\"");
    target_service_reg = _target_service_reg;
    // Get new session interface:
    auto erc = _target_service_reg.ifc.open_session(
                    _target_service_reg.ctx, "", &target_session_reg);
    if (erc)
        return erc;

    // Output welcome message, if defined:
    string welcome = server.get_welcome();
    if (!welcome.empty())
        output((const uint8_t*)welcome.c_str(), welcome.length());
    // Tell the server our reponse function:
    target_session_reg.ifc.get(target_session_reg.ctx, "", my_resp_fun, this);

    reader.reset(new thread([this] () { run(); }));
    reader->detach();

    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close: " + name());
    if (target_service_reg.ifc.close_session)
        target_service_reg.ifc.close_session(target_service_reg.ctx, target_session_reg.ctx);
    target_session_reg.ctx = nullptr;
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
    close();
}

void Session::output(const uint8_t* pb, const size_t cb)
{
    auto cbSent = ::send(fD, pb, cb, 0);
    if (cbSent != cb) {
        Plugin::error("Telnet " + name() + ": Error sending data");
        quit = true;
    }
}

void Session::input(const uint8_t* pb, const size_t cb)
{
    if (target_session_reg.ifc.post) {
        auto erc = target_session_reg.ifc.post(target_session_reg.ctx, "", pb, cb);
        if (erc)
            Plugin::error("Telnet session post() error: " + to_string(erc) + ": " +
                          BkBase::bk_error_message(erc));
    } else {
        Plugin::dump("Missed post", pb, cb);
    }
}
