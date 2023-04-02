#include "version.h"
#include "getopt.h"
#include "so.hpp"
#include "semaphore.hpp"
#include "service.hpp"
#include "cli.hpp"
#include "bk/module.h"
#include "bk/service.h"

#include <cstdlib>
#include <jsonx.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <exception>
#include <cassert>
#include <cstring>
#include <cctype>
#include <atomic>

using namespace std;
using namespace jsonx;

static atomic_bool quit{false};
static semaphore gate{};
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

static const lookup_t lookup_ifc {
    .find_service = [] (const char* server_name) -> const service_t*
    {
        auto service = Service::lookup(server_name);
        return service ? &service->service_ifc : nullptr;
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
        static void*  client_ctx{nullptr};
        static resp_f client_fun{nullptr};
        static session_t my_session_ifc {
            .get = [] (void* server_ctx, const char* head, resp_f fun) -> bk_error_t
            {
                if ((!client_ctx) || (server_ctx != (void*)1))
                    return BK_ERC_NO_SUCH_SESSION;
                client_fun = fun;
                stringstream oss;
                quit = !Cli::exec(head ? head : "", oss);
                if (client_fun) {
                    string response = oss.str();
                    client_fun(client_ctx, "", response.c_str(), response.length());
                }
                gate.notify();
                return BK_ERC_OK;
            },
            .post = [] (void* server_ctx, const char* head, const char* p_body, size_t c_body) -> bk_error_t
            {
                if ((!client_ctx) || (server_ctx != (void*)1))
                    return BK_ERC_NO_SUCH_SESSION;
                stringstream oss;
                quit = !Cli::exec(p_body ? string(p_body, c_body) : "", oss);
                if (client_fun) {
                    string response = oss.str();
                    client_fun(client_ctx, "", response.c_str(), response.length());
                }
                gate.notify();
                return BK_ERC_OK;
            }
        };
        service_t my_service_ifc {
            .open_session = []
                (void* client_loc_ctx, void** server_ctx_ptr, const char* meta, const session_t** ifc_ptr) -> bk_error_t
            {
                if (client_ctx)
                    return BK_ERC_TO_MUCH_SESSIONS;
                if (!ifc_ptr)
                    return BK_ERC_NO_SESSION_IFC_PTR;
                *ifc_ptr = &my_session_ifc;
                if (!server_ctx_ptr)
                    return BK_ERC_NO_SESSION_ID_PTR;
                *server_ctx_ptr = (void*)1;
                client_ctx = client_loc_ctx;
                return BK_ERC_OK;
            },
                .close_session = [] (void* server_ctx) -> bk_error_t
            {
                if ((!client_ctx) || (server_ctx != (void*)1))
                    return BK_ERC_NO_SUCH_SESSION;
                client_ctx = nullptr;
                client_fun = nullptr;
                return BK_ERC_OK;
            }
        };
        auto sys_service = Service::create(meta, nullptr, my_service_ifc); // Not loaded from SO
        // Administrator interface:
        admin_t admin_ifc {
            .publish = [] (const char*      _module_id,
                           const char*      _meta,
                           const service_t* _service_ifc)->bk_error_t
            {
                try {
                    assert(_module_id);
                    assert(_meta);
                    assert(_service_ifc);
                    json meta;
                    meta.parse(_meta);
                    string name = meta["name"];
                    auto module_ptr = SharedObject::lookup(_module_id);
                    if (!module_ptr)
                        throw runtime_error("Module not found: " + name);
                    if (!silent)
                        cout << "[d] Create service \"" << name << "\"" << endl;
                    Service::create(meta, module_ptr, *_service_ifc).get();
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
            },
            .dump = [] (const char *text, const char *pb, size_t cb)->void
            {
                cerr << text << ": ";
                if (!cb) {
                    cerr << endl;
                    return;
                }
                const auto MAX_L = 40UL; // Max no. of bytes in a line

                const char* _pb{pb};
                size_t _cb{cb};

                while (_cb) {
                    auto __pb = _pb;
                    auto l{min(MAX_L, _cb)};
                    for (auto i{l}; i > 0; --i, ++__pb) {
                        uint8_t x = static_cast<uint8_t>(*__pb);
                        cerr << hex << setw(2) << setfill('0')
                             << static_cast<int>(x) << " ";
                    } // end for //
                    cerr << "[";
                    __pb = _pb;
                    for (auto i{l}; i > 0; --i, ++__pb) {
                        char c = static_cast<char>(*__pb);
                        cerr << (isprint(c) ? c : '.');
                    } // end for //
                    cerr << "]" << endl;
                    _pb += l;
                    _cb -= l;
                    if (_cb)
                        for (auto i = strlen(text) + 2; i > 0; --i)
                            cerr << " ";
                } // end while //
            }
        };

        // Loop through the plugins list to load plugins;
        auto plugins = document["plugins"].toArray();
        for_each(plugins.begin(), plugins.end(),  [&] (json meta) {
            auto load = meta["load"];
            if ((!load.isBool()) || (load.toBool())) {
                string plugin_root = document["plugin_root"];
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
                    module_ptr->load(so->id.c_str(), &admin_ifc, oss.str().c_str());
                }
            }
        });

        // Loop through the plugins list to start plugins:
        for_each(SharedObject::container.begin(),
                 SharedObject::container.end(),  [] (auto &sop) {
            auto so = sop.second;
            bk_error_t erc = so->start(&lookup_ifc);
            if (erc != BK_ERC_OK)
                throw runtime_error("Error " + to_string(erc) +
                                    " when starting plugin \"" +
                                    so->get_name() +
                                    "\"");
        });

        // Loop through the launch list to start processes:
        auto launches = document["launch"].toArray();
        for_each(launches.begin(), launches.end(),  [&] (json meta) {
            string cmd = meta.toString();
            int result = ::system(cmd.c_str());
            admin_ifc.debug(BK_DEBUG,
                          ("Launch of \"" + cmd
                             + "\" exited with code "
                             + to_string(result)).c_str());
        });

        // Block until quit command was given:
        while (!quit)
            gate.wait();

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
