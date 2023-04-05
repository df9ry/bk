#ifndef _UDPSERVER_PLUGIN_HPP
#define _UDPSERVER_PLUGIN_HPP

#include <stdexcept>
#include <string>

#include <jsonx.hpp>

#include <bk/error.h>
#include <bk/module.h>
#include <bkbase/bkobject.hpp>

namespace UdpServer {

class PluginException: public std::runtime_error
{
public:
    PluginException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Plugin: public BkBase::BkObject {
public:
    static Plugin* self;

    // ID of the module:
    const std::string id;

    // Meta data:
    jsonx::json meta;

    static Plugin* constructor(
        const std::string& id, const admin_t *admin_ifc, const jsonx::json& meta)
    {
        self = new Plugin(id, admin_ifc, meta);
        return self;
    }

    static void debug(const std::string& msg) {
        self->admin_ifc.debug(BK_DEBUG, msg.c_str());
    }

    static void info(const std::string& msg) {
        self->admin_ifc.debug(BK_INFO, msg.c_str());
    }

    static void warning(const std::string& msg) {
        self->admin_ifc.debug(BK_WARNING, msg.c_str());
    }

    static void error(const std::string& msg) {
        self->admin_ifc.debug(BK_ERROR, msg.c_str());
    }

    static void fatal(const std::string& msg) {
        self->admin_ifc.debug(BK_FATAL, msg.c_str());
    }

    static void dump(const std::string& msg, const char* pb, size_t cb) {
        self->admin_ifc.dump(("udpserver:" + msg).c_str(), pb, cb);
    }

    const admin_t admin_ifc;

    ~Plugin() = default;

    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = delete;

    virtual std::string name() const { return meta["name"]; }
    bk_error_t publish_services();

private:
    Plugin(const std::string& _id, const admin_t *_admin_ifc, const jsonx::json& _meta):
        id{_id}, admin_ifc{*_admin_ifc}, meta{_meta}
    {}

};

} // end namespace UdpServer //

#endif // _UDPSERVER_PLUGIN_HPP