#ifndef AX25AUTOMATON_PRIMITIVE_HPP
#define AX25AUTOMATON_PRIMITIVE_HPP

#include <ax25base/ax25frame.hpp>

namespace AX25Automaton {

class AbstractPrimitive
{
public:
    enum class Kind_t {
        AX25Frame
    };

    AbstractPrimitive(Kind_t _kind): kind{_kind} {}

    const Kind_t kind;
};

template <typename T, AbstractPrimitive::Kind_t K>
class Primitive: public AbstractPrimitive
{
public:
    Primitive(T _ptr): AbstractPrimitive(K), ptr{_ptr} {}
    const T ptr;
};

typedef Primitive<AX25Base::AX25Frame::Ptr, AbstractPrimitive::Kind_t::AX25Frame>
    AX25FramePrimitive;

} // namespace AX25Automaton

#endif // AX25AUTOMATON_PRIMITIVE_HPP
