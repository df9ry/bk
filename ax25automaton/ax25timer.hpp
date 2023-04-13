#ifndef AX25AUTOMATON_AX25TIMER_HPP
#define AX25AUTOMATON_AX25TIMER_HPP

#include <bkbase/timer.hpp>

namespace AX25Automaton {

class AX25Timer: private Timer
{
public:
    AX25Timer();

    void start();
};

} // namespace AX25Automaton

#endif // AX25AUTOMATON_AX25TIMER_HPP
