#include "cli.hpp"
#include "so.hpp"
#include "service.hpp"

#include <string>
#include <regex>
#include <iostream>
#include <functional>

using namespace std;

vector<std::string> Cli::vec{};

void Cli::list()
{
    size_t l = vec.size();
    string s = "";
    while (l == 2) {
        s = vec[1];
        if (s == "plugins") {
            list_plugins();
            return;
        }
        if (s == "services") {
            list_services();
            return;
        }
        break;
    } // end while //
    cerr << "Not found: \"list " << s << "\"!" << endl
         << "  (\"list plugins\" | \"list services\")" << endl;
}

void Cli::list_plugins()
{
    for_each(SharedObject::container.begin(),
             SharedObject::container.end(),
             [] (const auto pair)
    {
        auto so = pair.second;
        cout << so->id << " " << so->get_name() << endl;
    });
}

void Cli::list_services()
{
    for_each(Service::container.begin(),
             Service::container.end(),
             [] (const auto pair)
    {
        auto service = pair.second;
        string plugin_name = service->get_plugin() ?
                    service->get_plugin()->get_name() : "";
        cout << service->get_name() << " (" << plugin_name << ")" << endl;
    });
}

void Cli::exec()
{
    while (true) {
        cout << "adm> ";
        cout.flush();
        string cmd;
        getline(cin, cmd);

        auto const re = regex{R"(\s+)"};
        vec = vector<string>(
            sregex_token_iterator{begin(cmd), end(cmd), re, -1},
            sregex_token_iterator{}
        );
        if (vec.size() == 0)
            continue;
        string s = vec[0];
        size_t l = vec.size();
        if ((l == 1) && (s == "quit"))
            break;
        if (s == "list") {
            list();
            continue;
        }
        cerr << "Not found: \"" << s << "\"!" << endl
             << "  (\"quit\" | \"list\")" << endl;
    } // end while //
}
