#include "ax25exceptions.hpp"

using namespace std;

namespace AX25Base {

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const string& what):
        std::runtime_error("Argument out of range: " + what)
    {};

    InvalidAX25FrameException::InvalidAX25FrameException(const string& what):
        std::runtime_error("Invalid AX25 frame: " + what)
    {};

    InvalidPropertyException::InvalidPropertyException(const string& what):
        std::runtime_error("Invalid property: " + what)
    {};

} // end namespace AX25Base //
