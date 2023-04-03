
#include "plugin.hpp"
#include "server.hpp"

#include "bk/module.h"

#include <jsonx.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <exception>

using namespace std;
using namespace jsonx;

Plugin* Plugin::self{nullptr};

bk_error_t Plugin::publish_services()
{
    assert(Plugin::self);
    auto services = Plugin::self->meta["services"].toArray();
    for_each(services.begin(), services.end(), [this] (json service) {
        Server::create(service);
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
            Plugin::info("Start plugin \"" + Plugin::self->get_name() + "\"");
            // Start all server:
            for_each(Server::container.begin(), Server::container.end(), [] (auto &pair)
            {
                auto server = pair.second;
                bk_error_t erc = server->start();
                if (erc != BK_ERC_OK)
                    Plugin::fatal("Start plugin \"" + Plugin::self->get_name() + "\" " +
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
                    Plugin::fatal("Stop plugin \"" + Plugin::self->get_name() + "\" " +
                         "failed with error code " + to_string(erc));
            });
            Plugin::info("Stop plugin \"" + Plugin::self->get_name() + "\"");
            return BK_ERC_OK;
        }
    };
} // end extern C //
