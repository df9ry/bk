#include "server.hpp"
#include "plugin.hpp"
#include "crcb.hpp"

#include <cstring>
#include <algorithm>
#include <cassert>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> // For close
#include <errno.h>

using namespace std;
using namespace jsonx;

static const session_t my_session_ifc {
    .get = [] (void* server_ctx, const char* head, resp_f fun) -> bk_error_t
    {
        return Server::self(server_ctx)->get(head, fun);
    },
    .post = [] (void* server_ctx, const char* head, const char* p_body, size_t c_body) -> bk_error_t
    {
        return Server::self(server_ctx)->post(head, p_body, c_body);
    }
};

Server::Map_t Server::container;

Server::Server(const json &_meta):
    meta{_meta}
{
    auto _crc = meta["crc"].toString();
    if ((_crc == "") || (_crc == "NONE"))
        crc_type = NONE;
    else if (_crc == "crcB")
        crc_type = CRC_B;
    else
        Plugin::fatal("UDPClient: Invalid CRC type " + _crc);
}

Server::~Server()
{
    if (info) {
        freeaddrinfo(info);
        info = nullptr;
    }
}

Server::Ptr_t Server::create(json meta)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Server(meta)).first->second;
}

bk_error_t Server::start(const lookup_t* _lookup_ifc)
{
    Plugin::info("Start server \"" + get_name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;
    if (crc_type == UNDEF)
        return BK_ERC_INV_CRC_TYPE;
    worker.reset(new thread([this] () { run(); }));
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop server \"" + get_name() + "\"");
    quit = true;
    ::close(sockFD);
    //worker->join();
    worker.release();
    return BK_ERC_OK;
}

void Server::run()
{
    // Start external TCP service:
    string port = meta["port"];
    if (port.empty())
        port = "telnet";
    string host = meta["host"];
    if (host.empty())
        host = "localhost";
    int ip_v = meta["ip-v"];
    // we need 2 pointers, res to hold and p to iterate over:
    addrinfo  hints;
    memset(&hints, 0, sizeof(hints));
    // for more explanation, man socket
    switch (ip_v) {
    case 0:
        hints.ai_family = AF_UNSPEC;
        break;
    case 4:
        hints.ai_family = AF_INET;
        break;
    case 6:
        hints.ai_family = AF_INET6;
        break;
    default:
        Plugin::error("Invalid internet version (ip-v) property: " +
                      to_string(ip_v) + "! Should be 4 or 6.");
        return;
    } // end switch //
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    // man getaddrinfo
    assert(!info);
    int gAddRes = getaddrinfo(host.c_str(), port.c_str(), &hints, &info);
    if (gAddRes != 0) {
        Plugin::error("Unable to get address info! Error: " +
                      string(gai_strerror(gAddRes)));
        return;
    }
    if (!info) {
        Plugin::error("No available adress found!");
        return;
    }

    string ipVer;
    if (info->ai_family == AF_INET) {
        ipVer              = "IPv4";
        sockaddr_in  *ipv4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        target_addr        = &(ipv4->sin_addr);
    } else {
        ipVer              = "IPv6";
        sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        target_addr        = &(ipv6->sin6_addr);
    }
    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(info->ai_family, target_addr, ipStr, sizeof(ipStr));

    Plugin::info("UDP client \"" + get_name() + "\" using " +
                 ipVer + ":" + ipStr + ":" + port);

    // let's create a new socket, socketFD is returned as descriptor
    // man socket for more information
    // these calls usually return -1 as result of some error
    assert(sockFD == -1);
    sockFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sockFD == -1) {
        int erc = errno;
        Plugin::fatal("UDP client: Error while creating socket: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }

    const int enable = 1;
    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        int erc = errno;
        Plugin::fatal("UDP client: setsockopt(SO_REUSEADDR) failed: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }

#if 0
    if (::connect(sockFD, (struct sockaddr *)target_addr, info->ai_addrlen) < 0) {
        int erc = errno;
        Plugin::fatal("UDP client: connect failed: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }
#endif

    // Receive UDP packages:
    while (!quit) {
        char buffer[1024];
        struct sockaddr rx_addr;
        socklen_t rx_addrlen;
        int n = recvfrom(sockFD, buffer, sizeof(buffer), 0,
                         &rx_addr, &rx_addrlen);
        Plugin::debug("UDP recvfrom n=" + to_string(n));
        if (quit)
            break;
        if (n < 0) {
            int erc = errno;
            Plugin::error("UDP client: recv failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            continue;
        }
        if (n > 0) {
            Plugin::dump("UDP RX", buffer, n);
            if (response_f) {
                switch (crc_type) {
                case NONE:
                    response_f(this, "", buffer, n);
                    break;
                case CRC_B:
                    if (n >= 2) {
                        auto crc = CrcB::crc((const uint8_t*)buffer, n-2);
                        if (((crc >> 8) == buffer[n-2]) && ((crc & 0x00ff) == buffer[n-1])) {
                            response_f(this, "", buffer, n-2);
                        } else {
                            Plugin::warning("Invalid CRC");
                        }
                    } else {
                        Plugin::warning("Received short package");
                    }
                    break;
                default:
                    assert(false);
                    break;
                } // end switch //
            }
        }
    } // end while //
    freeaddrinfo(info);
    info = nullptr;
}

bk_error_t Server::open_session(void** server_ctx_ptr, const char* meta, const session_t** ifc_ptr)
{
    if (session_connected)
        return BK_ERC_ENGAGED;
    if (server_ctx_ptr)
        *server_ctx_ptr = this;
    if (ifc_ptr)
        *ifc_ptr = &my_session_ifc;
    session_connected = true;
    Plugin::info("Session in " + get_name() + " opened");
    return BK_ERC_OK;
}

bk_error_t Server::close_session()
{
    if (session_connected) {
        Plugin::info("Session in " + get_name() + " closed");
        session_connected = false;
    }
    response_f = nullptr;
    return BK_ERC_OK;
}

bk_error_t Server::get(const char* head, resp_f fun)
{
    if (!session_connected)
        return BK_ERC_NOT_CONNECTED;
    response_f = fun;
    return BK_ERC_OK;
}

bk_error_t Server::post(const char* head, const char* p_body, size_t c_body)
{
    if (!session_connected)
        return BK_ERC_NOT_CONNECTED;
    int n;
    switch (crc_type) {
    case NONE:
        n = ::sendto(sockFD, (const uint8_t*)p_body, c_body, 0,
                     (struct sockaddr *)target_addr, info->ai_addrlen);
        break;
    case CRC_B: {
            auto p = (const uint8_t*)p_body;
            vector<uint8_t> frame(p, p+c_body);
            auto crc = CrcB::crc(p, c_body);
            frame.push_back(crc >> 8);
            frame.push_back(crc & 0x00ff);
            n = ::sendto(sockFD, frame.data(), frame.size(), 0,
                         (struct sockaddr *)target_addr, info->ai_addrlen);
        }
        break;
    default:
        assert(false);
        return BK_ERC_INV_CRC_TYPE;
    } // end switch //
    if (n == -1) {
        auto erc = errno;
        Plugin::error("sendto failed with erc " + to_string(erc) + "(" + strerror(erc) + ")");
        return BK_ERC_TALK_ERROR;
    }
    return BK_ERC_OK;
}
