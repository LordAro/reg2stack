#ifndef REGISTER_CONVERT_HPP
#define REGISTER_CONVERT_HPP

#include <vector>

#include "register_machine.hpp"
#include "stack_machine.hpp"

using prog_snippet = std::vector<j5::instruction>;

j5::program reg2stack(dcpu16::program p);
prog_snippet convert_instruction(const dcpu16::instruction &r);
template<typename It>
prog_snippet convert_instructions(It begin, It end)
{
	std::vector<prog_snippet> snippets;
	for (auto it = begin; it != end; ++it) {
		auto snippet = convert_instruction(*it);
		/* Deal with generated BRZERO from if statements
		 * requiring the length of the next instruction */
		if (snippets.size() != 0) {
			auto &last = snippets.back();
			auto &last_last = last.back();
			if (last_last.op.which() == 1 && boost::get<uint16_t>(last_last.op) == 42 &&
					(last_last.code == j5::op_t::BRZERO || last_last.code == j5::op_t::BRANCH)) {
				last_last.op = snippet.size() + 1;
			}
		}
		snippets.push_back(snippet);
	}
	/* Flatten */
	prog_snippet ret;
	for (const auto & snip : snippets) ret.insert(ret.end(), snip.begin(), snip.end());
	return ret;
}

prog_snippet index_on_stack(dcpu16::operand_t x);
prog_snippet address_on_stack(dcpu16::operand_t x);
prog_snippet value_on_stack(dcpu16::operand_t x);

#endif /* REGISTER_CONVERT_HPP */
