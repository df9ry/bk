#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"
#include "agw_struct.hpp"

#include <jsonx.hpp>

#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

static string char_2_string(char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return string(buf);
}

static string buf10_2_string(const char *buf10)
{
    char buf[11] {0};
    memcpy(buf, buf10, 10);
    buf[10] = '\0';
    return string(buf);
}

static string meta_2_string(const json &meta)
{
    stringstream oss;
    oss << meta;
    return oss.str();
}

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
        receive(buffer, n);
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

void Session::receive(const char* pb, size_t cb)
{
    rx_buffer.insert(rx_buffer.end(), pb, pb + cb);
    while (true) {
        auto size = rx_buffer.size();
        if (size < sizeof(agw_header_t)) {
            have_header = false;
            return;
        }
        if (!have_header) {
            frame_meta.clear();
            // OK, now we have a header. Map header struct to vector:
            auto header = (agw_header_t*)rx_buffer.data();
            frame_meta["port"] = static_cast<int>(header->port);
            frame_meta["kind"] = char_2_string(static_cast<char>(header->kind));
            frame_meta["from"] = buf10_2_string(header->call_from);
            frame_meta["to"]   = buf10_2_string(header->call_to);
            frame_meta["user"] = static_cast<int>(header->user);
            data_size = static_cast<size_t>(header->data_length);
            have_header = true;
        }
        if (size >= sizeof(agw_header_t) + data_size) {
            Plugin::dump("RX: " + meta_2_string(frame_meta), &rx_buffer[sizeof(agw_header_t)], data_size);
            rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + sizeof(agw_header_t) + data_size);
            have_header = false;
        }
    } // end while //
}
