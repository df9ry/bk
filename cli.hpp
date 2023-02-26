#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <vector>
#include <ostream>

class Cli
{
public:
    Cli() = delete; // Static class

    static bool exec(const std::string &cmd, std::ostream &os);

private:
    static std::vector<std::string> vec;

    static void list(std::ostream &os);
    static void list_plugins(std::ostream &os);
    static void list_services(std::ostream &os);
};

#endif // CLI_HPP
