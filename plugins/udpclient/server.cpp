#include "server.hpp"
#include "plugin.hpp"

#include <cstring>
#include <algorithm>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> // For close

using namespace std;
using namespace jsonx;

Server::Map_t Server::container;

Server::Server(const json &_meta):
    meta{_meta}
{}

Server::~Server()
{
    if (info) {
        freeaddrinfo(info);
        info = nullptr;
    }
    session = nullptr;
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

    string target = meta["target"];
    if (!target.empty()) {
        if (!lookup_ifc.find_service)
            Plugin::fatal("lookup_ifc.find_service is null");
        assert(lookup_ifc.find_service);
        auto _target_service_ifc = lookup_ifc.find_service(target.c_str());
        if (!_target_service_ifc)
            Plugin::fatal("Target not found: \"" + target + "\"");
        assert(_target_service_ifc);
        target_service_ifc = *_target_service_ifc;
    }

    worker.reset(new thread([this] () { run(); }));
    return BK_ERC_OK;
}

bk_error_t Server::stop()
{
    Plugin::info("Stop server \"" + get_name() + "\"");
    quit = true;
    if (session)
        session->get()->close();
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

    void* addr;
    string ipVer;
    if (info->ai_family == AF_INET) {
        ipVer              = "IPv4";
        sockaddr_in  *ipv4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        addr               = &(ipv4->sin_addr);
    } else {
        ipVer              = "IPv6";
        sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        addr               = &(ipv6->sin6_addr);
    }
    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(info->ai_family, addr, ipStr, sizeof(ipStr));

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
    if (::connect(sockFD, (struct sockaddr *)addr, info->ai_addrlen) < 0) {
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
        }
    } // end while //
    freeaddrinfo(info);
    info = nullptr;
}

void Server::close(Session* session)
{
    session = nullptr;
}
