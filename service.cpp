#include "service.hpp"
#include "cli.hpp"
#include "semaphore.hpp"

#include <sstream>

using namespace std;
using namespace jsonx;

extern bool quit;
extern semaphore gate;

static bool have_client{false};

static session_t my_session_ifc {
    // Get data from server, no request data:
    .get = [] (int         session_id, // from open_session()
               const char* request_meta,
               response_f  response_function, void* user_data) -> bk_error_t
    {
        if ((!have_client) || (session_id != 1))
            return BK_ERC_NO_SUCH_SESSION;
        stringstream oss;
        quit = !Cli::exec(request_meta, oss);
        gate.notify();
        if (response_function) {
            string response = oss.str();
            response_function(1, "{}", response.c_str(), response.length(), user_data);
        }
        return BK_ERC_OK;
    },
    .put = [] (int         session_id,
               const char* request_meta,
               const char* p_request_body,
               size_t      c_request_body) -> bk_error_t
    {
        if ((!have_client) || (session_id != 1))
            return BK_ERC_NO_SUCH_SESSION;
        stringstream oss;
        quit = !Cli::exec(string(p_request_body, c_request_body), oss);
        gate.notify();
        return BK_ERC_OK;
    },
    .xch = [] (int         session_id,
               const char* request_meta,
               const char* p_request_body,
               size_t      c_request_body,
               response_f  response_function, void* user_data) -> bk_error_t
    {
        if ((!have_client) || (session_id != 1))
            return BK_ERC_NO_SUCH_SESSION;
        stringstream oss;
        quit = !Cli::exec(string(p_request_body, c_request_body), oss);
        gate.notify();
        if (response_function) {
            string response = oss.str();
            response_function(1, "{}", response.c_str(), response.length(), user_data);
        }
        return BK_ERC_OK;
    }
};

Service::Map_t Service::container;

Service::Service(const json &_meta, SharedObject::Ptr_t _so):
    meta{_meta}, so{_so},
    service_ifc {
        .open_session = [] (const char*      meta,
                            session_t**      remote_session_ifc_ptr,
                            int*             remote_session_id_ptr) -> bk_error_t
        {
            if (have_client)
                return BK_ERC_TO_MUCH_SESSIONS;
            if (!remote_session_ifc_ptr)
                return BK_ERC_NO_SESSION_IFC_PTR;
            *remote_session_ifc_ptr = &my_session_ifc;
            if (!remote_session_id_ptr)
                return BK_ERC_NO_SESSION_ID_PTR;
            *remote_session_id_ptr = 1;
            have_client = true;
            return BK_ERC_OK;
        },
        .close_session = [] (int session_id) -> bk_error_t
        {
            if ((!have_client) || (session_id != 1))
                return BK_ERC_NO_SUCH_SESSION;
            have_client = false;
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

