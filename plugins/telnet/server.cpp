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
    int backlog = meta["backlog"];
    // number of connections allowed on the incoming queue:
    if (!backlog)
        backlog = 8;
    // we need 2 pointers, res to hold and p to iterate over:
    addrinfo  hints;
    addrinfo *info;
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
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    // man getaddrinfo
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

    Plugin::info("Telnet server \"" + get_name() + "\" using " +
                 ipVer + ":" + ipStr + ":" + port);

    // let's create a new socket, socketFD is returned as descriptor
    // man socket for more information
    // these calls usually return -1 as result of some error
    int sockFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sockFD == -1) {
        Plugin::fatal("Error while creating socket");
        freeaddrinfo(info);
        return;
    }

    // Let's bind address to our socket we've just created
    int bindR = bind(sockFD, info->ai_addr, info->ai_addrlen);
    if (bindR == -1) {
        Plugin::fatal("Error while binding socket");
        ::close(sockFD);
        freeaddrinfo(info);
        return;
    }

    // finally start listening for connections on our socket
    int listenR = listen(sockFD, backlog);
    if (listenR == -1) {
        Plugin::fatal("Error while Listening on socket\n");
        ::close(sockFD);
        freeaddrinfo(info);
        return;
    }

    freeaddrinfo(info);

    // structure large enough to hold client's address
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    // a fresh infinite loop to communicate with incoming connections
    // this will take client connections one at a time
    // in further examples, we're going to use fork() call for each client connection
    while (true) {
        // accept call will give us a new socket descriptor
        int newFD
          = accept(sockFD, (sockaddr *) &client_addr, &client_addr_size);
        if (newFD == -1) {
            Plugin::error("Error while Accepting on socket");
            continue;
        }
        auto session_ptr = Session::create(*this, newFD, ++session_id);
        // Find a slot in the sessions container or create a new one:
        auto iter = find_if(sessions.begin(), sessions.end(),
                            [](const auto &sp)->bool { return (!sp); });
        if (iter == sessions.end())
            sessions.push_back(session_ptr);
        else
            *iter = session_ptr;
        // Start session:
        session_ptr->open(target_service_ifc);
    }
}

void Server::close(Session* session)
{
    auto iter = find_if(sessions.begin(), sessions.end(),
                    [session](const auto &sp)->bool { return (session == sp.get()); });
    if (iter != sessions.end())
        *iter = nullptr;
}
