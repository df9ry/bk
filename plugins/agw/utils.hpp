#ifndef UTILS_HPP
#define UTILS_HPP

#include "agw_struct.hpp"

#include <string>
#include <cstring>
#include <sstream>
#include <jsonx.hpp>

static inline std::string char_2_string(char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return std::string(buf);
}

static inline std::string buf10_2_string(const char *buf10)
{
    char buf[11];
    ::memcpy(buf, buf10, 10);
    buf[10] = '\0';
    return std::string(buf);
}

static inline void buf10_set(char* buf10, const std::string& s)
{
    ::memcpy(buf10, s.c_str(), std::min<size_t>(s.length(), 10));
}

static inline std::string meta_2_string(const jsonx::json &meta)
{
    std::stringstream oss;
    oss << meta;
    return oss.str();
}

static inline agw_frame_kind string_2_kind(const std::string &s)
{
    return static_cast<agw_frame_kind>((s.length() == 1) ? s[0] : ' ');
}

#endif // UTILS_HPP
