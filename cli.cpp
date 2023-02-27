#include "cli.hpp"
#include "so.hpp"
#include "service.hpp"

#include <string>
#include <regex>
#include <iostream>
#include <functional>

using namespace std;

vector<std::string> Cli::vec{};

void Cli::list(std::ostream &os)
{
    size_t l = vec.size();
    string s = "";
    while (l == 2) {
        s = vec[1];
        if (s == "plugins") {
            list_plugins(os);
            return;
        }
        if (s == "services") {
            list_services(os);
            return;
        }
        break;
    } // end while //
    os << "Not found: \"list " << s << "\"!" << endl
       << "  (\"list plugins\" | \"list services\")" << endl;
}

void Cli::list_plugins(std::ostream &os)
{
    for_each(SharedObject::container.begin(),
             SharedObject::container.end(),
             [&os] (const auto pair)
    {
        auto so = pair.second;
        os << so->id << " " << so->get_name() << endl;
    });
}

void Cli::list_services(std::ostream &os)
{
    for_each(Service::container.begin(),
             Service::container.end(),
             [&os] (const auto pair)
    {
        auto service = pair.second;
        string plugin_name = service->get_plugin() ?
                    service->get_plugin()->get_name() : "";
        os << service->get_name() << " (" << plugin_name << ")" << endl;
    });
}

bool Cli::exec(const string &cmd, ostream &os)
{
    auto const re = regex{R"(\s+)"};
    vec = vector<string>(
        sregex_token_iterator{begin(cmd), end(cmd), re, -1},
        sregex_token_iterator{}
    );
    bool ok = true;
    if (vec.size() > 0) {
        string s = vec[0];
        if (!s.empty()) {
            size_t l = vec.size();
            if ((l == 1) && (s == "quit")) {
                ok = false;
            } else if (s == "list") {
                list(os);
            } else {
                os << "Not found: \"" << s << "\"!" << endl
                   << "  (\"quit\" | \"list\")" << endl;
            }
        }
    }
    os << "adm> ";
    return ok;
}
