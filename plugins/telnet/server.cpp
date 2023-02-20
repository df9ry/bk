#include "server.hpp"

using namespace std;
using namespace jsonx;

Server::Map_t Server::container;

Server::Server(const json &_meta):
    meta{_meta},
    session_admin_ifc {
        .open_session = [] (const char* meta,
                                session_t** session_ifc_ptr,
                                int* session_id_ptr)->bk_error_t
        {
            if (!session_ifc_ptr)
                return BK_ERC_NO_SESSION_IFC_PTR;
            //TODO: Create a session
            *session_ifc_ptr = nullptr;
            if (!session_id_ptr)
                return BK_ERC_NO_SESSION_ID_PTR;
            *session_id_ptr = -1;
            return BK_ERC_OK;
        },
        .close_session = [] (int session_id)->bk_error_t
        {
            return BK_ERC_OK;
        }
    }
{
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

const Server& Server::create_server(json meta) {
    string name = meta["name"];
    if (container.contains(name))
        throw ServerException("Service already defined: " + name);
    return *container.emplace(name, new Server(meta)).first->second;
}
