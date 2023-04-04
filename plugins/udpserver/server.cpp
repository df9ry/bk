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

namespace UdpServer {

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
    Plugin::info("Start server \"" + name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;
    if (crc_type == UNDEF)
        return BK_ERC_INV_CRC_TYPE;
    worker.reset(new thread([this] () { run(); }));
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop server \"" + name() + "\"");
    quit = true;
    ::close(sockFD);
    //worker->join();
    worker.release();
    return BK_ERC_OK;
}

void Server::run()
{
    // Start external TCP service:
    auto port = meta["port"].toInt();
    if (!port)
        Plugin::fatal("Missing port property");
    string host = meta["host"];
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
        Plugin::fatal("Invalid internet version (ip-v) property: " +
                      to_string(ip_v) + "! Should be 4 or 6.");
        return;
    } // end switch //
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    // man getaddrinfo
    assert(!info);
    int gAddRes = getaddrinfo(host.c_str(), to_string(port).c_str(), &hints, &info);
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
    void  *listen_addr;
    if (info->ai_family == AF_INET) {
        ipVer              = "IPv4";
        sockaddr_in  *ipv4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        listen_addr        = &(ipv4->sin_addr);
    } else {
        ipVer              = "IPv6";
        sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        listen_addr        = &(ipv6->sin6_addr);
    }
    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(info->ai_family, listen_addr, ipStr, sizeof(ipStr));

    Plugin::info("UDP server \"" + name() + "\" using " +
                 ipVer + ":" + ipStr + ":" + to_string(port));

    // let's create a new socket, socketFD is returned as descriptor
    // man socket for more information
    // these calls usually return -1 as result of some error
    assert(sockFD == -1);
    sockFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sockFD == -1) {
        int erc = errno;
        Plugin::fatal("UDP server: Error while creating socket: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }

    const int enable = 1;
    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        int erc = errno;
        Plugin::fatal("UDP server: setsockopt(SO_REUSEADDR) failed: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }

    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port);
    serAddr.sin_addr.s_addr = host.empty() ? INADDR_ANY : inet_addr(host.c_str());

    if (::bind(sockFD, (struct sockaddr*)&serAddr, sizeof(serAddr)) < 0) {
        int erc = errno;
        Plugin::fatal("UDP server bind failed: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return;
    }

    // Receive UDP packages:
    while (!quit) {
        char buffer[1024];
        socklen_t cliAddrLen = sizeof(cliAddr);

        struct sockaddr rx_addr;
        socklen_t rx_addrlen;
        int n = recvfrom(sockFD, buffer, sizeof(buffer), 0,
                         (struct sockaddr*)&cliAddr, &cliAddrLen);
        Plugin::debug("UDP server recvfrom n=" + to_string(n));
        if (quit)
            break;
        if (n < 0) {
            int erc = errno;
            Plugin::error("UDP server: recv failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            continue;
        }
        if (n > 0) {
            Plugin::dump("UDP server RX", buffer, n);
#if 0
            if (response_fun) {
                switch (crc_type) {
                case NONE:
                    response_fun(response_ctx, "", buffer, n);
                    break;
                case CRC_B:
                    if (n >= 2) {
                        auto crc = CrcB::crc((const uint8_t*)buffer, n-2);
                        if (((crc >> 8) == buffer[n-2]) && ((crc & 0x00ff) == buffer[n-1])) {
                            response_fun(response_ctx, "", buffer, n-2);
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
#endif
        }
    } // end while //
    freeaddrinfo(info);
    info = nullptr;
}

} // end namespace UdpServer //
