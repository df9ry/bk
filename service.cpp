#include "service.hpp"
#include "cli.hpp"
#include "semaphore.hpp"

#include <sstream>

using namespace std;
using namespace jsonx;

extern bool quit;
extern semaphore gate;

static void*  client_ctx{nullptr};
static resp_f client_fun{nullptr};

static session_t my_session_ifc {
    .get = [] (void* server_ctx, const char* head, resp_f fun) -> bk_error_t
    {
        if ((!client_ctx) || (server_ctx != (void*)1))
            return BK_ERC_NO_SUCH_SESSION;
        client_fun = fun;
        stringstream oss;
        quit = !Cli::exec(head ? head : "", oss);
        if (client_fun) {
            string response = oss.str();
            client_fun(client_ctx, "", response.c_str(), response.length());
        }
        gate.notify();
        return BK_ERC_OK;
    },
    .post = [] (void* server_ctx, const char* head, const char* p_body, size_t c_body) -> bk_error_t
    {
        if ((!client_ctx) || (server_ctx != (void*)1))
            return BK_ERC_NO_SUCH_SESSION;
        stringstream oss;
        quit = !Cli::exec(p_body ? string(p_body, c_body) : "", oss);
        if (client_fun) {
            string response = oss.str();
            client_fun(client_ctx, "", response.c_str(), response.length());
        }
        gate.notify();
        return BK_ERC_OK;
    }
};

Service::Map_t Service::container;

Service::Service(const json &_meta, SharedObject::Ptr_t _so):
    meta{_meta}, so{_so},
    service_ifc {
        .open_session = []
            (void* client_loc_ctx, void** server_ctx_ptr, const char* meta, session_t** ifc_ptr) -> bk_error_t
        {
            if (client_ctx)
                return BK_ERC_TO_MUCH_SESSIONS;
            if (!ifc_ptr)
                return BK_ERC_NO_SESSION_IFC_PTR;
            *ifc_ptr = &my_session_ifc;
            if (!server_ctx_ptr)
                return BK_ERC_NO_SESSION_ID_PTR;
            *server_ctx_ptr = (void*)1;
            client_ctx = client_loc_ctx;
            return BK_ERC_OK;
        },
        .close_session = [] (void* server_ctx) -> bk_error_t
        {
            if ((!client_ctx) || (server_ctx != (void*)1))
                return BK_ERC_NO_SUCH_SESSION;
            client_ctx = nullptr;
            client_fun = nullptr;
            return BK_ERC_OK;
        }
    }
{
}

Service::~Service()
{
}

Service::Ptr_t Service::create(json meta, SharedObject::Ptr_t _so)
{
    string name = meta["name"];
    if (container.contains(name))
        return nullptr;
    return container.emplace(name, new Service(meta, _so)).first->second;
}

const Service& Service::create_service(json meta, SharedObject::Ptr_t _so) {
    string name = meta["name"];
    if (container.contains(name))
        throw ServiceException("Service already defined: " + name);
    return *container.emplace(name, new Service(meta, _so)).first->second;
}

