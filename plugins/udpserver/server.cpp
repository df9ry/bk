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

#define ECHO_TEST // Simply echo received data back

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
    Plugin::info("Start UDP server \"" + name() + "\"");
    assert(_lookup_ifc);
    lookup_ifc = *_lookup_ifc;
    if (crc_type == UNDEF)
        return BK_ERC_INV_CRC_TYPE;
    // Start external TCP service:
    auto port = meta["port"].toString();
    if (port.empty())
        Plugin::fatal("UDP server: Missing port property");
    auto host = meta["host"].toString();
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
        Plugin::error("UDP server: Invalid internet version (ip-v) property: " +
                      to_string(ip_v) + "! Should be 4 or 6.");
        return BK_ERC_INV_ADDR_INFO;
    } // end switch //
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    // man getaddrinfo
    struct addrinfo *info;
    int erc = getaddrinfo(host.empty() ? nullptr : host.c_str(),
                          port.c_str(), &hints, &info);
    if (erc) {
        Plugin::fatal("UDP server: Unable to get address info! Error: " +
                      string(gai_strerror(erc)));
        return BK_ERC_INV_ADDR_INFO;
    }
    if (!info) {
        Plugin::fatal("UDP server: No available adress found!");
        return BK_ERC_INV_ADDR_INFO;
    }
    // Find usable port:
    for(struct addrinfo *addr = info; addr != NULL; addr = addr->ai_next)
    {
        sockFD = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sockFD == -1)
        {
            erc = errno;
            continue;
        }
        const int enable = 1;
        if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            int erc = errno;
            Plugin::fatal("UDP server: setsockopt(SO_REUSEADDR) failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            freeaddrinfo(info);
            info = nullptr;
            return BK_ERC_INV_ADDR_INFO;
        }
        if (::bind(sockFD, addr->ai_addr, addr->ai_addrlen) == 0)
            break;
        erc = errno;
        close(sockFD);
        sockFD = -1;
    } // end for //
    if (erc) {
        Plugin::fatal("UDP server: Unable to find usable interface! Error: " +
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
    Plugin::info("UDP server \"" + name() + "\" using " +
                 ipVer + ":" + ipStr + ":" + port);
    if (sockFD == -1) {
        int erc = errno;
        Plugin::fatal("UDP server: Error while creating socket: " +
                      to_string(erc) + " (" + strerror(erc) + ")");
        freeaddrinfo(info);
        info = nullptr;
        return BK_ERC_INV_ADDR_INFO;
    }
    freeaddrinfo(info);
    info = nullptr;
    assert(sockFD != -1);
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
    // Receive UDP packages:
    while (!quit) {
        uint8_t buffer[1024];
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
            Plugin::error("UDP server: recvfrom failed: " +
                          to_string(erc) + " (" + strerror(erc) + ")");
            continue;
        }
        if (n > 0) {
            //Plugin::dump("UDP server RX", buffer, n);
#ifdef ECHO_TEST
            //Plugin::dump("UDP server TX", buffer, n);
            int l = ::sendto(sockFD, buffer, n, 0,
                             (struct sockaddr*)&cliAddr, cliAddrLen);
            if (l < 0) {
                int erc = errno;
                Plugin::error("UDP server: sendto failed: " +
                              to_string(erc) + " (" + strerror(erc) + ")");
            }
#else
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
}

} // end namespace UdpServer //
