#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <vector>

class Cli
{
public:
    Cli() = delete; // Static class

    static void exec();

private:
    static std::vector<std::string> vec;

    static void list();
    static void list_plugins();
    static void list_services();
};

#endif // CLI_HPP
