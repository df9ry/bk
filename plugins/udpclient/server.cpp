#include "server.hpp"
#include "plugin.hpp"

#include <bkbase/crcb.hpp>

#include <cstring>
#include <algorithm>
#include <cassert>
#include <vector>
#include <exception>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> // For close
#include <errno.h>

using namespace std;
using namespace jsonx;

namespace UdpClient {

static const session_t my_session_ifc {
    .get = [] (void* session_ctx, const char* head, resp_f fun, void* ctx) -> bk_error_t
    {
        try {
            return BkBase::self<Server>(session_ctx).get(head, fun, ctx);
        }
        catch (const exception& ex) {
            Plugin::error(string("UdpClient::get() exception: ") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    },
    .post = [] (void* session_ctx, const char* head, const uint8_t* p_body, size_t c_body) -> bk_error_t
    {
        try {
            return BkBase::self<Server>(session_ctx).post(head, p_body, c_body);
        }
        catch (const exception& ex) {
            Plugin::error(string("UdpClient::post() exception: ") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    }
};

Server::Map_t Server::container;

Server::Server(const json &_meta):
    BkBase::BkObject(), meta{_meta}
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
    Plugin::info("Start UDP client \"" + name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;
    if (crc_type == UNDEF)
        return BK_ERC_INV_CRC_TYPE;
    // Start external TCP service:
    auto port = meta["port"].toString();
    if (port.empty())
        Plugin::fatal("UDP client: Missing port property");
    auto host = meta["host"].toString();
    if (host.empty())
        Plugin::fatal("UDP client: Missing host property");
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
        Plugin::error("UDP client: Invalid internet version (ip-v) property: " +
                      to_string(ip_v) + "! Should be 4 or 6.");
        return BK_ERC_INV_ADDR_INFO;
    } // end switch //
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    // man getaddrinfo
    struct addrinfo *info;
    int erc = getaddrinfo(host.c_str(), port.c_str(), &hints, &info);
    if (erc) {
        Plugin::fatal("UDP client: Unable to get address info! Error: " +
                      string(gai_strerror(erc)));
        return BK_ERC_INV_ADDR_INFO;
    }
    if (!info) {
        Plugin::fatal("UDP client: No available adress found!");
        return BK_ERC_INV_ADDR_INFO;
    }
    // Find usable port:
    for(struct addrinfo *addr = info; addr != NULL; addr = addr->ai_next)
    {
        sockFD = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sockFD == -1)
        {
            erc = errno;
            continue;
        }
        const int enable = 1;
        if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            int erc = errno;
            Plugin::fatal("UDP client: setsockopt(SO_REUSEADDR) failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            freeaddrinfo(info);
            info = nullptr;
            return BK_ERC_INV_ADDR_INFO;
        }
        if (::connect(sockFD, addr->ai_addr, addr->ai_addrlen) == 0)
            break;
        erc = errno;
        ::close(sockFD);
        sockFD = -1;
    } // end for //
    if (erc) {
        Plugin::fatal("UDP client: Unable to find usable interface! Error: " +
                      string(gai_strerror(erc)));
        freeaddrinfo(info);
        return BK_ERC_INV_ADDR_INFO;
    }
    // Display interface info:
    string ipVer;
    void *peer_addr{nullptr};
    if (info->ai_family == AF_INET) {
        ipVer              = "IPv4";
        sockaddr_in  *ipv4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        peer_addr          = &(ipv4->sin_addr);
    } else {
        ipVer              = "IPv6";
        sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        peer_addr          = &(ipv6->sin6_addr);
    }
    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(info->ai_family, peer_addr, ipStr, sizeof(ipStr));
    Plugin::info("UDP client \"" + name() + "\" using " +
                 ipVer + ":" + ipStr + ":" + port);
    if (sockFD == -1) {
        int erc = errno;
        Plugin::fatal("UDP client: Error while creating socket: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return BK_ERC_INV_ADDR_INFO;
    }
    freeaddrinfo(info);
    assert(sockFD != -1);
    worker.reset(new thread([this] () { run(); }));
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop UDP client \"" + name() + "\"");
    quit = true;
    ::close(sockFD);
    //worker->join();
    worker.release();
    return BK_ERC_OK;
}

void Server::run()
{
    // Receive UDP packages:
    while (!quit) {
        struct sockaddr_in serAddr;
        socklen_t serAddrLen = sizeof(serAddr);
        uint8_t buffer[1024];
        //int n = recvfrom(sockFD, buffer, sizeof(buffer), 0,
        //                 (struct sockaddr*)&serAddr, &serAddrLen);
        int n = recv(sockFD, buffer, sizeof(buffer), MSG_WAITALL);
        Plugin::debug("UDP client recv n=" + to_string(n));
        if (quit)
            break;
        if (n < 0) {
            int erc = errno;
            Plugin::error("UDP client: recv failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            continue;
        }
        if (n > 0) {
            Plugin::dump("UDP client RX", buffer, n);
            if (response_fun) {
                switch (crc_type) {
                case NONE:
                    response_fun(response_ctx, "", buffer, n);
                    break;
                case CRC_B:
                    if (n >= 2) {
                        uint16_t crc = CrcB::crc((const uint8_t*)buffer, n-2);
                        if ((static_cast<uint8_t>(crc >> 8)     == buffer[n-1]) &&
                            (static_cast<uint8_t>(crc & 0x00ff) == buffer[n-2]))
                        {
                            response_fun(response_ctx, "", buffer, n-2);
                        } else {
                            Plugin::warning("UDP client: Invalid CRC");
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
}

bk_error_t Server::open_session(const char* meta, session_reg_t* reg)
{
    if (session_connected)
        return BK_ERC_ENGAGED;
    *reg = session_reg_t{ my_session_ifc, this };
    session_connected = true;
    Plugin::info("Session in " + name() + " opened");
    return BK_ERC_OK;
}

bk_error_t Server::close_session(const void* session_ctx)
{
    if (session_connected) {
        Plugin::info("Session in " + name() + " closed");
        session_connected = false;
    }
    response_fun = nullptr;
    response_ctx = nullptr;
    return BK_ERC_OK;
}

bk_error_t Server::get(const char* head, resp_f fun, void* ctx)
{
    if (!session_connected)
        return BK_ERC_NOT_CONNECTED;
    response_fun = fun;
    response_ctx = ctx;
    return BK_ERC_OK;
}

bk_error_t Server::post(const char* head, const uint8_t* p_body, size_t c_body)
{
    if (!session_connected)
        return BK_ERC_NOT_CONNECTED;
    int n;
    switch (crc_type) {
    case NONE:
        n = ::send(sockFD, p_body, c_body, 0);
        break;
    case CRC_B: {
            auto p = (const uint8_t*)p_body;
            vector<uint8_t> frame(p, p+c_body);
            uint16_t crc = CrcB::crc(p, c_body);
            frame.push_back(static_cast<uint8_t>(crc & 0x00ff));
            frame.push_back(static_cast<uint8_t>(crc >> 8));
            n = ::send(sockFD, frame.data(), frame.size(), 0);
        }
        break;
    default:
        assert(false);
        return BK_ERC_INV_CRC_TYPE;
    } // end switch //
    if (n == -1) {
        auto erc = errno;
        Plugin::error("UDP client: send failed with erc "
                      + to_string(erc) + "(" + strerror(erc) + ")");
        return BK_ERC_TALK_ERROR;
    }
    return BK_ERC_OK;
}

} // end namespace UdpClient //
