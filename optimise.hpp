#ifndef OPTIMISE_HPP
#define OPTIMISE_HPP

#include "stack_machine.hpp"

j5::program peephole_optimise(j5::program prog);
j5::program stack_schedule(j5::program prog);

#endif /* OPTIMISE_HPP */
