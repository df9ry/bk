
#include "plugin.hpp"

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
        stringstream oss;
        service.write(oss);
#if 0

        bk_error_t erc = service_ifc.publish(id.c_str(), oss.str().c_str());
        if (erc != BK_ERC_OK)
            service_ifc.debug(BK_FATAL, (
                                  "Unable to publish service \"" +
                                  service["name"].toString() +
                                  "\"!. ERC = " +
                                  to_string(erc)).c_str());
#endif
    }); // end for_each //
    return BK_ERC_OK;
}

extern "C" {
    module_t module {
        .load  = [] (const char *id, const service_t *sys, const char *meta)->bk_error_t
        {
            if (!id)
                return BK_ERC_NO_ID;
            if (!sys)
                return BK_ERC_NO_SERVICE_IFC;
            if ((!sys->debug) || (!sys->publish) || (!sys->withdraw))
                return BK_ERC_INV_SERVICE_IFC;
            if (!meta)
                return BK_ERC_NO_META;
            try {
                json _meta;
                _meta.parse(meta);
                if (!_meta.isObject())
                    return BK_ERC_INV_META;
                return Plugin::constructor(id, sys, _meta)->publish_services();
            }
            catch(const runtime_error &ex) {
                sys->debug(BK_FATAL, ex.what());
                return BK_ERC_PUBLISH;
            }
            return BK_ERC_OK;
        },
        .start = [] ()->bk_error_t { return BK_ERC_OK; },
        .stop  = [] ()->bk_error_t { return BK_ERC_OK; }
    };
} // end extern C //
