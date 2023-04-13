#ifndef AX25AUTOMATON_STATE_HPP
#define AX25AUTOMATON_STATE_HPP

namespace AX25Automaton {

enum class State
{
    Exit,
    DataLinkDisconnected,
    AwaitingConnection,
    Connected,
};

} // namespace AX25Automaton

#endif // AX25AUTOMATON_STATE_HPP
