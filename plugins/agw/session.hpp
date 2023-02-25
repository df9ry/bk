#ifndef SESSION_HPP
#define SESSION_HPP

#include <bk/error.h>

#include <jsonx.hpp>

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <set>

class Server;

class Session
{
public:
    typedef std::shared_ptr<Session> Ptr_t;

    static Ptr_t create(Server& server, int fD, int id);

    ~Session();

    Session() = delete;
    Session(const Session& other) = delete;
    Session(Session&& other) = delete;

    std::string       name() const;

    const int         id;
    Server&           server;
    std::atomic<bool> quit{true};

    bk_error_t        open(const jsonx::json &meta);
    void              close();

    bool              use_raw_frames() const { return raw_frames; }
    bool              do_monitor() const { return monitor; }

private:
    Session(Server& server, int fD, int id);

    const std::atomic<int>       fD;
    jsonx::json                  meta{};

    std::unique_ptr<std::thread> reader{nullptr};
    std::vector<char>            rx_buffer{};
    bool                         have_header{false};
    size_t                       data_size{0};
    jsonx::json                  frame_meta{};
    bool                         raw_frames{false};
    bool                         monitor{false};

    void                         run();
    void                         transmit(const char* pb, const size_t cb);
    void                         receive(const char* pb, size_t cb);
    void                         receive(const jsonx::json &meta,
                                         const char* pb, size_t cb);
    void                         register_call(const std::string &call);
    void                         unregister_call(const std::string &call);
    void                         version();
    void                         port_info();
    std::set<std::string>        calls{};
};

#endif // SESSION_HPP
