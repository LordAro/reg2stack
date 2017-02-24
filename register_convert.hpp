#ifndef REGISTER_CONVERT_HPP
#define REGISTER_CONVERT_HPP

#include <vector>

#include "register_machine.hpp"
#include "stack_machine.hpp"

using prog_snippet = std::vector<j5::instruction>;

j5::program reg2stack(dcpu16::program p);

#endif /* REGISTER_CONVERT_HPP */
