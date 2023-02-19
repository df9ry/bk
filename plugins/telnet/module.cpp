
#include <bk/module.h>
#include <bk/service.h>

#include <jsonx.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <exception>

using namespace std;
using namespace jsonx;

// Interface to system service structure:
static const service_t* sys = nullptr;

// ID of the module:
static string module_id;

// Meta data:
static json module_meta;

static void publish_services()
{
    auto services = ::module_meta["services"].toArray();
    for_each(services.begin(), services.end(), [] (json service) {
        stringstream oss;
        service.write(oss);
        ::sys->publish(::module_id.c_str(), oss.str().c_str(), nullptr);
    }); // end for_each //
}

extern "C" {
    module_t module {
        .load  = [] (const char      *_id,
                     const service_t *_sys, 
                     const char      *_meta)
        {
            assert(_id);
            ::module_id = _id;
            assert(_sys);
            assert(_sys->publish);
            assert(_sys->withdraw);
            assert(_sys->debug);
            ::sys = _sys;
            assert(_meta);
            try {
                ::module_meta.parse(_meta);
            }
            catch(const runtime_error &ex) {
                ::sys->debug(BK_FATAL, ex.what());
            }
            ::sys->debug(BK_INFO, "Loading module telnet");
            ::publish_services();
        },
        .start = [] () {}, 
        .stop  = [] () {}
    };
} // end extern C //
