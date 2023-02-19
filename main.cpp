#include "version.h"
#include "getopt.h"
#include "so.hpp"
#include "service.hpp"
#include "bk/module.h"
#include "bk/service.h"

#include <jsonx.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <exception>

using namespace std;
using namespace jsonx;

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
       << "\t-c <config file> ............ Configuration file" << endl
       << "\t-h .......................... Print help (this message)" << endl
       << "\t-s .......................... Silent" << endl;
}

int main(int argc, char** argv) {
    int   option{0};
    bool  silent{false};

    filesystem::path cwd = filesystem::path(argv[0]).parent_path();
    filesystem::path config_file_name = cwd.append("daisy.conf");

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
        const Service& sys_service = Service::create_service(document["meta"]);
        service_t sys{
            .publish = [] (const char *meta)->void* {
                 return Service::create(meta).get();
            },
            .withdraw = [] (const char *name)->bool {
                return Service::remove_service(name);
            }
        };
        // Loop through the service list to load plugins;
        string plugin_root = document["plugin_root"];
        auto plugins = document["plugins"].toArray();
        for_each(plugins.begin(), plugins.end(),  [&] (json meta) {
            filesystem::path path = meta["path"].toString();
            filesystem::path plugin_path = filesystem::weakly_canonical(
                plugin_root.append("/").append(path)
            );
            if (!silent)
                cout << "[i] Loading " << plugin_path << endl;
            auto so = SharedObject::create(plugin_path, meta);
            // Bind and call the load method of the plugin:
            const module_t* module = static_cast<module_t*>(so->getsym("module"));
            if (!module)
                throw runtime_error("Import error in " 
                        + string(plugin_path.c_str()) 
                        + ": Error: " + so->error_text());
            if (module->load)
                module->load(so->id.c_str(), &sys); 
        });
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
