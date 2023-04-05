#include "session.hpp"
#include "server.hpp"
#include "plugin.hpp"
#include "agw_struct.hpp"
#include "utils.hpp"

#include <jsonx.hpp>

#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

Session::Session(Server& _server, int _fD, int _id):
    server{_server}, fD{_fD}, id{_id}
{
}

Session::~Session()
{
    Plugin::info("Close agw session \"" + name() + "\"");
    ::close(fD);
}

Session::Ptr_t Session::create(Server& server, int fD, int id)
{
    return Ptr_t(new Session(server, fD, id));
}

string Session::name() const
{
    return server.get_name() + "/" + to_string(id);
}

bk_error_t Session::open(const json &_meta)
{
    Plugin::info("Open agw session \"" + name() + "\"");
    meta = _meta;
    reader.reset(new thread([this] () { run(); }));
    reader->detach();
    return BK_ERC_OK;
}

void Session::close()
{
    Plugin::debug("Close agw session \"" + name() + "\"");
    server.close(this);
}

void Session::run()
{
    uint8_t buffer[256];
    quit = false;
    while (!quit) {
        int n = ::recv(fD, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            Plugin::debug("Ret: " + to_string(n));
            break;
        }
        receive(buffer, n);
        //Plugin::dump("RX raw:", buffer, n);
    } // end while //
    server.close(this);
}

void Session::transmit(const uint8_t* pb, const size_t cb)
{
    Plugin::dump("TX", pb, cb);
    auto cbSent = ::send(fD, pb, cb, 0);
    if (cbSent != cb) {
        Plugin::error("AGW " + name() + ": Error sending data");
        quit = true;
    }
}

void Session::receive(const uint8_t* pb, size_t cb)
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
            receive(frame_meta, &rx_buffer[sizeof(agw_header_t)], data_size);
            rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + sizeof(agw_header_t) + data_size);
            have_header = false;
        }
    } // end while //
}

void Session::receive(const json& meta, const uint8_t* pb, size_t cb)
{
    Plugin::dump("RX" + meta_2_string(meta), pb, cb);
    switch (string_2_kind(meta["kind"])) {
    case VERSION:
        version();
        return;
    case RAW_SWITCH:
        raw_frames = !raw_frames;
        Plugin::debug("Agw: Set raw frames: " + string(raw_frames ? "on" : "off"));
        return;
    case MONITOR:
        monitor = !monitor;
        Plugin::debug("Agw: Set monitor: " + string(monitor ? "on" : "off"));
        return;
    case REGISTER_CALL:
        register_call(meta["from"]);
        return;
    case UNREGISTER_CALL:
        unregister_call(meta["from"]);
        return;
    case PORT_INFO:
        port_info();
        return;
    default:
        break;
    } // end switch //
    int port_no = meta["port"];
    if (port_no >= server.ports.size()) {
        Plugin::error("Received message for undefined port "
                      + to_string(port_no));
        Plugin::dump(meta_2_string(meta), pb, cb);
        return;
    }
    server.ports.at(port_no)->receive(*this, meta, pb, cb);
}

void Session::register_call(const string &call)
{
    uint8_t result;
    if (calls.contains(call)) {
        Plugin::warning("Agw: Call " + call + " already registered");
        result = 0;
    } else {
        Plugin::debug("Agw: Register call " + call);
        calls.insert(call);
        result = 1;
    }
    struct __attribute__((__packed__)) port_info_reply_t {
        agw_header_t header;
        uint8_t      result;
    };
    union __attribute__((__packed__)) {
        port_info_reply_t structured;
        uint8_t flat[sizeof(port_info_reply_t)];
    } frame;
    ::memset(frame.flat, 0x00, sizeof(frame.flat));
    frame.structured.header.kind = REGISTER_CALL;
    frame.structured.header.data_length = sizeof(uint8_t);
    buf10_set(frame.structured.header.call_from, call);
    frame.structured.result = result;
    transmit(frame.flat, sizeof(frame.flat));
}

void Session::unregister_call(const string &call)
{
    uint8_t result;
   if (calls.contains(call)) {
        Plugin::debug("Agw: Unregister call " + call);
        calls.extract(call);
        result = 1;
    } else {
        Plugin::warning("Agw: Call " + call + " not registered");
        result = 0;
    }
   struct __attribute__((__packed__)) port_info_reply_t {
       agw_header_t header;
       uint8_t      result;
   };
   union __attribute__((__packed__)) {
       port_info_reply_t structured;
       uint8_t flat[sizeof(port_info_reply_t)];
   } frame;
   ::memset(frame.flat, 0x00, sizeof(frame.flat));
   frame.structured.header.kind = UNREGISTER_CALL;
   frame.structured.header.data_length = sizeof(uint8_t);
   buf10_set(frame.structured.header.call_from, call);
   frame.structured.result = result;
   transmit(frame.flat, sizeof(frame.flat));
}

void Session::port_info()
{
    Plugin::debug("Agw: Asking for port info");
    string info;
    {
        stringstream ss;
        ss << server.ports.size();
        for_each(server.ports.begin(), server.ports.end(), [&ss] (const auto &port) {
            string description = port->meta["description"];
            replace( description.begin(), description.end(), ';', '~');
            ss << ";" << description;
        }); // end for_each //
        ss << ";";
        info = ss.str();
    }
    vector<uint8_t> frame;
    {
        agw_header_t header;
        ::memset(&header, 0x00, sizeof(header));
        header.kind = PORT_INFO;
        header.data_length = info.length();
        char *p_header = (char*)&header;
        frame.insert(frame.end(), p_header, p_header + sizeof(header));
    }
    frame.insert(frame.end(), info.begin(), info.end());
    transmit(frame.data(), frame.size());
}

void Session::version()
{
    int major = meta["ver_major"];
    int minor = meta["ver_minor"];
    Plugin::debug("Agw: Asking for version: " + to_string(major) + "." + to_string(minor));
    struct __attribute__((__packed__)) port_info_reply_t {
        agw_header_t header;
        uint32_t     major;
        uint32_t     minor;
    };
    union __attribute__((__packed__)) {
        port_info_reply_t structured;
        uint8_t flat[sizeof(port_info_reply_t)];
    } frame;
    ::memset(frame.flat, 0x00, sizeof(frame.flat));
    frame.structured.header.kind = VERSION;
    frame.structured.header.data_length = 2 * sizeof(uint32_t);
    frame.structured.major = major;
    frame.structured.minor = minor;
    transmit(frame.flat, sizeof(frame.flat));
}
