#include <iostream>
#include <thread>

#include "register_convert.hpp"
#include "stackconvert_machine.hpp"
#include "util.hpp"

void convertmachine::run_reg(const dcpu16::program &prog, bool verbose, bool speedlimit)
{
	this->terminate = false;
	bool skip_next = false;
	this->snippet_cache.resize(prog.size());
	this->reg_prog = prog;
	this->pc = 0;
	for (; !this->terminate && this->pc < prog.size(); this->pc++) {
		if (skip_next) {
			skip_next = false;
			continue;
		}
		auto start = std::chrono::high_resolution_clock::now();


		auto snippet = this->snippet_cache.at(this->pc);
		if (snippet.empty()) {
			std::cerr << "Caching " << prog.at(this->pc) << '\n';
			snippet = convert_instruction(prog.at(this->pc));
			this->snippet_cache.at(this->pc) = snippet;
		}

		std::cerr << prog.at(this->pc) << '\n';
		for (const auto &i : snippet) {
			std::cerr << '\t' << i << '\n';
			// Override BRZERO
			if (i.code == j5::op_t::BRZERO
					&& HasBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO))) {
				assert(boost::get<uint16_t>(i.op) == 2);
				skip_next = true;
				continue;
			}
			auto new_pc = this->run_instruction(i);
			switch (i.code) {
				case j5::op_t::BRANCH:
					this->pc = new_pc - 1;
					break;
				default:
					break;
			}
		}

		/* Deal with generated BRZERO from if statements
		 * requiring the length of the next instruction */
		/*if (reg_pc > 0) {
			auto &last = prog.at(reg_pc - 1);
			if (dcpu16::is_if_code(last.code)) {

				brzero_idx->op = snippet.size() + 1 + 2;
			}
			auto brzero_idx = std::find_if(last.begin(), last.end(),
					[](const j5::instruction &i){return i.code == j5::op_t::BRZERO;});
		}
		// TODO: Look at p instead of snippets?
		if (snippets.size() != 0) {
			auto &last = snippets.back();
			auto brzero_idx = std::find_if(last.begin(), last.end(),
					[](const j5::instruction &i){return i.code == j5::op_t::BRZERO;});
			if (brzero_idx != last.end()) {
				brzero_idx->op = snippet.size() + 1 + 2;
			}
		}*/

		if (verbose) std::cout << this->register_dump() << '\n';
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

