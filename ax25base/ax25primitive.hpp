#ifndef AX25BASE_AX25PRIMITIVE_HPP
#define AX25BASE_AX25PRIMITIVE_HPP

#include <string>

namespace AX25Base {

/// <summary>
/// A primitive that can be sent along a channel.
/// </summary>
class AX25Primitive
{
public:
    AX25Primitive();

    /// <summary>
    /// Get a human readable string that can be used in a monitor.
    /// </summary>
    virtual std::string to_string() const = 0;
};

} // namespace AX25Base

#endif // AX25BASE_AX25PRIMITIVE_HPP
