#include "version.h"
#include "getopt.h"
#include "so.hpp"
#include "cli.hpp"
#include "service.hpp"
#include "bk/module.h"
#include "bk/service.h"

#include <cstdlib>
#include <jsonx.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <exception>
#include <cassert>
#include <cstring>

using namespace std;
using namespace jsonx;

static bool silent{false};

static void print_version(ostream &os)
{
    os << "[i] This is " << APP_NAME << " version " << APP_VERSION
       << ", Copyright (C) by " << APP_COPYRIGHT << endl
       << "[i] - see: " << APP_WEBSITE
       << endl;
}

static void help(ostream &os, const char *name)
{
    print_version(os);
    os << "Usage: " << name << endl
       << "\t-c <config file> .. Configuration file" << endl
       << "\t-h ................ Print help (this message)" << endl
       << "\t-s ................ Silent" << endl;
}

static int is_sys_session_open{false};

static session_t sys_session {
    .get = [] (int         session_id,
               const char* request_meta,
               response_f  response_function, void* user_data)->bk_error_t
    {
        return BK_ERC_NOT_IMPLEMENTED;
    },
    .put = [] (int         session_id,
               const char* request_meta,
               uint8_t*    p_request_body,
               size_t      c_request_body)->bk_error_t
    {
        return BK_ERC_NOT_IMPLEMENTED;
    },
    .xch = [] (int         session_id,
               const char* request_meta,
               uint8_t*    p_request_body,
               size_t      c_request_body,
               response_f  response_function, void* user_data)->bk_error_t
    {
        return BK_ERC_NOT_IMPLEMENTED;
    }
};

static session_admin_t sys_sap {
    .open_session = [] (const char* meta,
                        session_t** session_ifc_ptr,
                        int* session_id_ptr)->bk_error_t
    {
        if (is_sys_session_open)
            return BK_ERC_TO_MUCH_SESSIONS;
        if (!session_ifc_ptr)
            return BK_ERC_NO_SESSION_IFC_PTR;
        is_sys_session_open = true;
        *session_ifc_ptr = &sys_session;
        if (!session_id_ptr)
            return BK_ERC_NO_SESSION_ID_PTR;
        *session_id_ptr = 0;
        return BK_ERC_OK;
    },
    .close_session = [] (int session_id)->bk_error_t
    {
        if (is_sys_session_open) {
            is_sys_session_open = false;
            return BK_ERC_OK;
        } else {
            return BK_ERC_NO_SUCH_SESSION;
        }
    }
};

int main(int argc, char** argv) {
    int   option{0};

    filesystem::path cwd = filesystem::path(argv[0]).parent_path();
    filesystem::path config_file_name = cwd.append("conf.json");

    // Get options:
    while ((option = getopt(argc, argv, "c:hv")) >= 0) {
        switch (option) {
        case 'c':
            config_file_name = optarg;
            break;
        case 'h':
            help(cout, argv[0]);
            break;
        case 's':
            silent = true;
            break;
        default:
            help(cerr, argv[0]);
            return EXIT_FAILURE;
        } // end switch //
    } // end while //

    try {
        if (!silent)
            print_version(cout);
        
        // Read config file:
        if (!filesystem::exists(config_file_name))
            throw runtime_error("File " 
                                        + string(config_file_name.filename().c_str()) 
                                        + " not found");
        ifstream ifs;
        json document;
        if (!silent)
            cout << "[i] Reading config file " << config_file_name << endl;
        ifs.open(config_file_name);
        document.parse(ifs);
        if (!document.isObject())
            throw runtime_error("Config root is not an object");
        string configuration_name = document["name"];
        if (!silent)
            cout << "[i] Config name: \"" << configuration_name << "\"" << endl;

        // Create service "sys":
        json meta;
        meta["name"] = "sys";
        auto sys_service = Service::create(meta, nullptr, &sys_sap);
        service_t sys_ifc {
            .publish = [] (const char*            _module_id,
                           const char*            _meta,
                           const session_admin_t* _session_admin_ifc)->bk_error_t
            {
                try {
                    assert(_module_id);
                    assert(_meta);
                    json meta;
                    meta.parse(_meta);
                    string name = meta["name"];
                    auto module_ptr = SharedObject::lookup(_module_id);
                    if (!module_ptr)
                        throw runtime_error("Module not found: " + name);
                    if (!silent)
                        cout << "[d] Create service \"" << name << "\"" << endl;
                    Service::create(meta, module_ptr, _session_admin_ifc).get();
                    return BK_ERC_OK;
                } catch (exception &ex) {
                    cerr << "Unable to create service: " << ex.what() << endl;
                    exit(EXIT_FAILURE);
                }
            },
            .withdraw = [] (const char *module_id, const char *name)->bk_error_t
            {
                return Service::remove_service(name) ? BK_ERC_OK : BK_ERC_NO_SUCH_SERVICE;
            },
            .debug = [] (grade_t grade, const char *msg)->void
            {
                if (grade == BK_FATAL)
                    throw runtime_error(msg);
                if ((::silent) && (grade == BK_DEBUG))
                    return;
                auto &stream = ((grade == 'w') || (grade == 'e')) ? cerr : cout;
                stream << "[" << static_cast<char>(grade) << "] " << msg << endl;
            }
        };

        // Loop through the plugins list to load plugins;
        string plugin_root = document["plugin_root"];
        auto plugins = document["plugins"].toArray();
        for_each(plugins.begin(), plugins.end(),  [&] (json meta) {
            filesystem::path path = meta["path"].toString();
            filesystem::path plugin_path = filesystem::weakly_canonical(
                plugin_root.append("/").append(path)
            );
            if (!silent)
                cout << "[i] Load " << plugin_path << endl;
            auto so = SharedObject::create(plugin_path, meta);
            // Bind and call the load method of the plugin:
            const module_t* module_ptr = static_cast<module_t*>(so->getsym("module"));
            if (!module_ptr)
                throw runtime_error("Import error in " 
                        + string(plugin_path.c_str()) 
                        + ": Error: " + so->error_text());
            so->module_ifc = *module_ptr;
            if (module_ptr->load) {
                ostringstream oss;
                meta.write(oss);
                module_ptr->load(so->id.c_str(), &sys_ifc, oss.str().c_str());
            }
        });

        // Loop through the plugins list to start plugins;
        for_each(SharedObject::container.begin(),
                 SharedObject::container.end(),  [] (auto &sop) {
            auto so = sop.second;
            bk_error_t erc = so->start();
            if (erc != BK_ERC_OK)
                throw runtime_error("Error " + to_string(erc) +
                                    " when starting plugin \"" +
                                    so->get_name() +
                                    "\"");
        });

        Cli::exec();

        // Loop through the plugins list to stop plugins;
        for_each(SharedObject::container.rbegin(),
                 SharedObject::container.rend(),  [] (auto &sop) {
            auto so = sop.second;
            bk_error_t erc = so->stop();
            if (erc != BK_ERC_OK)
                throw runtime_error("Error " + to_string(erc) +
                                    " when stopping plugin \"" +
                                    so->get_name() +
                                    "\"");
        });
        Service::container.clear();
        SharedObject::container.clear();
    }
    catch (exception &ex) {
        cerr << "[e] Error: " << ex.what() << "!" << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "[e] Error: Unidentified failure" << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
