#include <iostream>
#include <thread>

#include "convert_machine.hpp"
#include "optimise.hpp"
#include "register_convert.hpp"
#include "util.hpp"

/* Get cached instruction snippet, if it exists. Otherwise create it */
std::pair<j5::program, uint16_t> convertmachine::get_snippet(uint16_t reg_pc, size_t optimise)
{
	auto section_it = this->section_cache.find(reg_pc);
	if (section_it != this->section_cache.end()) {
		return section_it->second;
	}

	// find end of section (next label)
	auto next_label = std::find_if(
		this->reg_prog.begin() + reg_pc + 1,
		this->reg_prog.end(),
		[](const dcpu16::instruction &i){return !i.label.empty();}
	);
	uint16_t distance = std::distance(this->reg_prog.begin() + reg_pc, next_label);
	log<LOG_DEBUG2>("# Caching ", this->reg_prog.at(reg_pc), " (",  distance, ")");
	j5::program snippet = convert_instructions(this->reg_prog.begin() + reg_pc, next_label);
	if (optimise >= 1) {
		snippet = peephole_optimise(snippet);
	}
	if (optimise >= 2) { // koopman
		snippet = stack_schedule(snippet);
		snippet = peephole_optimise(snippet); // peephole again
	}
	this->section_cache[reg_pc] = {snippet, distance};
	return this->section_cache[reg_pc];
}

void convertmachine::run_reg(const dcpu16::program &prog, bool speedlimit, size_t optimise, bool cache)
{
	this->terminate = false;
	this->reg_prog = prog;
	size_t program_cost = 0;
	size_t skip = 0;
	this->pc = 0;
	for (uint16_t reg_pc = 0; !this->terminate && reg_pc < this->reg_prog.size();) {
		auto start = std::chrono::high_resolution_clock::now();

		bool is_cached = cache && this->section_cache.find(reg_pc) != this->section_cache.end();
		j5::program snippet;
		uint16_t distance;
		std::tie(snippet, distance) = get_snippet(reg_pc, optimise);

		if (is_cached) {
			program_cost += 1; // lookup cost
		} else {
			program_cost += distance * 10; // caching cost
		}

		size_t memcount = std::count_if(snippet.begin(), snippet.end(), [](const auto &i){return i.code == j5::op_t::LOAD || i.code == j5::op_t::STORE;});
		log<LOG_DEBUG>(prog.at(reg_pc), "(size: ", snippet.size(), ", ", memcount, ")");

		/* Run instruction snippet */
		for (size_t start_pc = this->pc;  this->pc - start_pc < snippet.size(); this->pc++) {
			if (skip > 0) {
				skip--;
				continue;
			}
			const auto &i = snippet.at(this->pc - start_pc);
			log<LOG_DEBUG>('\t', i);
			auto new_pc = this->run_instruction(i);

			bool breakout = false;
			log<LOG_DEBUG2>(this->register_dump());
			// branch specials
			switch (i.code) {
				case j5::op_t::BRZERO:
					skip = new_pc - this->pc - 1; // get rid of relative
					break;
				case j5::op_t::BRANCH: {
					if (i.op.which() == 1) {
						skip = new_pc - this->pc - 1;
					} else {
						std::string branch_label = boost::get<std::string>(i.op);
						if (snippet.begin()->label == branch_label) {
							// label is in current snippet
							this->pc = start_pc - 1; // loop
						} else {
							reg_pc = new_pc - distance; // postinc
							breakout = true;
						}
					}
					break;
				}
				default:
					break;
			}

			switch (i.code) {
				case j5::op_t::BRZERO:
				case j5::op_t::BRANCH:
					program_cost += 2; // branch cost
					break;
				case j5::op_t::LOAD:
				case j5::op_t::STORE:
					program_cost += 3;
					break;
				default:
					program_cost += 1;
					break;
			}
			if (breakout) break;
		}
		log<LOG_DEBUG>("");

		if (speedlimit) {
			std::this_thread::sleep_until(start + (std::chrono::milliseconds(100) * distance)); // arbitrary
		}
		reg_pc += distance;
	}
	log<LOG_DEBUG>("Program cost: ", program_cost);
}

uint16_t convertmachine::find_label(const std::string &l)
{
	auto pos = std::find_if(this->reg_prog.begin(), this->reg_prog.end(), [l](const dcpu16::instruction &i) { return i.label == l; });
	if (pos == this->reg_prog.end()) {
		throw "Undefined label '" + l + "' used";
	}
	return std::distance(this->reg_prog.begin(), pos);
}

