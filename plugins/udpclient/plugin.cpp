
#include "plugin.hpp"
#include "server.hpp"

#include "bk/module.h"
#include "bkbase/bkerror.hpp"

#include <jsonx.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <exception>

using namespace std;
using namespace jsonx;

namespace UdpClient {

static const service_t my_service_ifc {
    .open_session = [] (void* server_ctx, const char* meta, session_reg_t* reg) -> bk_error_t
    {
        try {
            return BkBase::self<Server>(server_ctx).open_session(meta, reg);
        }
        catch (const exception& ex) {
            Plugin::error(string("UdpClient::Plugin::open_session() exception") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    },
    .close_session = [] (void* server_ctx, const void* session_ctx) -> bk_error_t
    {
        try {
            return BkBase::self<Server>(server_ctx).close_session(session_ctx);
        }
        catch (const exception& ex) {
            Plugin::error(string("UdpClient::Plugin::close_session() exception") + ex.what());
            return BK_ERC_RUNTIME_EXCEPTION;
        }
    }
};

Plugin* Plugin::self{nullptr};

bk_error_t Plugin::publish_services()
{
    assert(Plugin::self);
    auto services = Plugin::self->meta["services"].toArray();
    for_each(services.begin(), services.end(), [this] (json service) {
        stringstream oss;
        service.write(oss);
        auto p_server = Server::create(service);
        assert(p_server);
        const Server& server = *p_server;
        auto name = service["name"].toString();
        bk_error_t erc = admin_ifc.publish(
            id.c_str(), name.c_str(), oss.str().c_str(), service_reg_t { my_service_ifc, p_server.get() });
        if (erc != BK_ERC_OK)
            admin_ifc.debug(BK_FATAL, (string("Unable to publish service \"")
                                        + name + "\"!. ERC = "
                                        + to_string(erc)
                                        + ": "
                                        + BkBase::bk_error_message(erc)).c_str());
    }); // end for_each //
    return BK_ERC_OK;
}

extern "C" {
    module_t module {
        .load  = [] (const char *id,
                     const admin_t *admin,
                     const char *meta)->bk_error_t
        {
            if (!id)
                return BK_ERC_NO_ID;
            if (!admin)
                return BK_ERC_NO_SERVICE_IFC;
            if ((!admin->debug) || (!admin->publish) || (!admin->withdraw))
                return BK_ERC_INV_SERVICE_IFC;
            if (!meta)
                return BK_ERC_NO_META;
            try {
                json _meta;
                _meta.parse(meta);
                if (!_meta.isObject())
                    return BK_ERC_INV_META;
                return Plugin::constructor(id, admin, _meta)->publish_services();
            }
            catch(const runtime_error &ex) {
                Plugin::fatal(ex.what());
                return BK_ERC_PUBLISH;
            }
            return BK_ERC_OK;
        },
        .start = [] (const lookup_t* lookup_ifc)->bk_error_t {
        Plugin::info("Start plugin \"" + Plugin::self->name() + "\"");
            // Start all server:
            for_each(Server::container.begin(), Server::container.end(),
                     [lookup_ifc] (auto &pair)
            {
                auto server = pair.second;
                bk_error_t erc = server->start(lookup_ifc);
                if (erc != BK_ERC_OK)
                    Plugin::fatal("Start plugin \"" + Plugin::self->name() + "\" " +
                         "failed with error code " + to_string(erc));
            });
            return BK_ERC_OK;
        },
        .stop  = [] ()->bk_error_t {
            // Stop all server:
            for_each(Server::container.rbegin(), Server::container.rend(),
                     [] (auto &pair)
            {
                auto server = pair.second;
                bk_error_t erc = server->stop();
                if (erc != BK_ERC_OK)
                    Plugin::fatal("Stop plugin \"" + Plugin::self->name() + "\" " +
                         "failed with error code " + to_string(erc));
            });
            Plugin::info("Stop plugin \"" + Plugin::self->name() + "\"");
            return BK_ERC_OK;
        }
    };
} // end extern C //

} // end namespace UdpClient //

