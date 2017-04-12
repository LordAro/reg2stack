#ifndef STACKCONVERT_MACHINE_HPP
#define STACKCONVERT_MACHINE_HPP

#include <map>
#include <string>

#include "register_machine.hpp"
#include "stack_machine.hpp"

class convertmachine : j5::machine {
public:
	void run_reg(const dcpu16::program &prog, bool verbose, bool speedlimit);
private:
	uint16_t find_label(const std::string &l) override;
	dcpu16::program reg_prog;

	std::map<size_t, std::pair<j5::program, uint16_t>> section_cache;
};

#endif /* STACKCONVERT_MACHINE_HPP */
