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
            const std::string& id, const service_t *ifc, const jsonx::json& meta)
    {
        self = new Plugin(id, ifc, meta);
        return self;
    }

    ~Plugin() = default;

    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = delete;

    bk_error_t publish_services();

private:
    Plugin(const std::string& _id, const service_t *_ifc, const jsonx::json& _meta):
        id{_id}, service_ifc{*_ifc}, meta{_meta}
    {}

    service_t service_ifc{};
};

#endif // _PLUGIN_HPP
