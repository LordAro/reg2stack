#include <iostream>
#include <thread>

#include "register_convert.hpp"
#include "stackconvert_machine.hpp"
#include "util.hpp"

void convertmachine::run_reg(const dcpu16::program &prog, bool verbose, bool speedlimit)
{
	this->terminate = false;
	this->reg_prog = prog;
	size_t skip = 0;
	this->pc = 0;
	while (!this->terminate && this->pc < prog.size()) {
		auto start = std::chrono::high_resolution_clock::now();

		std::vector<prog_snippet> snippet;
		auto section_it = this->section_cache.find(this->pc);
		if (section_it != this->section_cache.end()) {
			snippet = section_it->second;
		} else {
			// find end of section (next label)
			auto next_label = std::find_if(
				this->reg_prog.begin() + this->pc + 1,
				this->reg_prog.end(),
				[](const dcpu16::instruction &i){return !i.label.empty();}
			);
			if (verbose) {
				std::cout << "# Caching "
				          << this->reg_prog.at(this->pc)
				          << "(+"
			              << std::distance(this->reg_prog.begin() + this->pc, next_label)
				          << ")\n";
			}
			snippet = convert_instructions(this->reg_prog.begin() + this->pc, next_label);
			this->section_cache[this->pc] = snippet;
		}

		std::cerr << prog.at(this->pc) << '\n';
		for (size_t start_pc = this->pc; this->pc - start_pc < snippet.size(); this->pc++) {
			const auto &snip = snippet.at(this->pc - start_pc);
			for (const auto &i : snip) {
				if (skip > 0) {
					skip--;
					continue;
				}
				std::cerr << '\t' << i << '\n';
				// Override BRZERO
				if (i.code == j5::op_t::BRZERO
						&& HasBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO))) {
					ClrBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
					skip = boost::get<uint16_t>(i.op) - 1;
					continue;
				}

				auto new_pc = this->run_instruction(i);

				if (verbose) std::cout << this->register_dump() << '\n';
				// branch specials
				if (i.code == j5::op_t::BRANCH) {
					this->pc = new_pc - 1; // postinc
					break;
				}
			}
			if (verbose) std::cout << '\n';
		}
		if (verbose) std::cout << '\n';

		if (speedlimit) {
			std::this_thread::sleep_until(start + std::chrono::milliseconds(100)); // arbitrary
		}
	}
}

uint16_t convertmachine::find_label(const std::string &l)
{
	auto pos = std::find_if(this->reg_prog.begin(), this->reg_prog.end(), [l](const dcpu16::instruction &i) { return i.label == l; });
	if (pos == this->reg_prog.end()) {
		throw "Undefined label '" + l + "' used";
	}
	return std::distance(this->reg_prog.begin(), pos);
}

