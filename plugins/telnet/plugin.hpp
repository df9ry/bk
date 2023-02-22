#ifndef _PLUGIN_HPP
#define _PLUGIN_HPP

#include <stdexcept>
#include <string>

#include <jsonx.hpp>

#include <bk/error.h>
#include <bk/service.h>
#include <bk/session.h>

class PluginException: public std::runtime_error
{
public:
    PluginException(const std::string &msg): std::runtime_error(msg.c_str()) {}
};

class Plugin {
public:
    static Plugin* self;

    // ID of the module:
    const std::string id;

    // Meta data:
    jsonx::json meta;

    static Plugin* constructor(
        const std::string& id, const service_t *sys_ifc, const jsonx::json& meta)
    {
        self = new Plugin(id, sys_ifc, meta);
        return self;
    }

    static void debug(const std::string& msg) {
        self->sys_ifc.debug(BK_DEBUG, msg.c_str());
    }

    static void info(const std::string& msg) {
        self->sys_ifc.debug(BK_INFO, msg.c_str());
    }

    static void warning(const std::string& msg) {
        self->sys_ifc.debug(BK_WARNING, msg.c_str());
    }

    static void error(const std::string& msg) {
        self->sys_ifc.debug(BK_ERROR, msg.c_str());
    }

    static void fatal(const std::string& msg) {
        self->sys_ifc.debug(BK_FATAL, msg.c_str());
    }

    const service_t sys_ifc;

    ~Plugin() = default;

    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = delete;

    std::string get_name() const { return meta["name"].toString(); }
    bk_error_t publish_services();

private:
    Plugin(const std::string& _id, const service_t *_sys_ifc, const jsonx::json& _meta):
        id{_id}, sys_ifc{*_sys_ifc}, meta{_meta}
    {}

};

#endif // _PLUGIN_HPP
