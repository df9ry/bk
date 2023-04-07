#ifndef _AX25BASE_AX25EXCEPTIONS_HPP
#define _AX25BASE_AX25EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace AX25Base {

    class ArgumentOutOfRangeException: public std::runtime_error {
    public:
        ArgumentOutOfRangeException(const std::string& what);
    };

    class InvalidAX25FrameException: public std::runtime_error {
    public:
        InvalidAX25FrameException(const std::string& what);
    };

    class InvalidPropertyException: public std::runtime_error {
    public:
        InvalidPropertyException(const std::string& what);
    };

    class NullPointerException: public std::runtime_error {
    public:
        NullPointerException(const std::string& what);
    };

} // end namespace AX25Base //

#endif // _AX25BASE_AX25EXCEPTIONS_HPP
