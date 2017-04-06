#ifndef STACKCONVERT_MACHINE_HPP
#define STACKCONVERT_MACHINE_HPP

#include <string>

#include "register_machine.hpp"
#include "stack_machine.hpp"

class convertmachine : j5::machine {
public:
	void run_reg(const dcpu16::program &prog, bool verbose, bool speedlimit);
private:
	uint16_t find_label(const std::string &l) override;
	dcpu16::program reg_prog;
};

#endif /* STACKCONVERT_MACHINE_HPP */
