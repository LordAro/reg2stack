#ifndef REGISTER_CONVERT_HPP
#define REGISTER_CONVERT_HPP

#include <vector>

#include "register_machine.hpp"
#include "stack_machine.hpp"

using prog_snippet = std::vector<j5::instruction>;

j5::program reg2stack(dcpu16::program p);
prog_snippet convert_instruction(const dcpu16::instruction &r);
template<typename It>
std::vector<prog_snippet> convert_instructions(It begin, It end)
{
	std::vector<prog_snippet> snippets;
	for (auto it = begin; it != end; ++it) {
		auto snippet = convert_instruction(*it);
		/* Deal with generated BRZERO from if statements
		 * requiring the length of the next instruction */
		if (snippets.size() != 0) {
			auto &last = snippets.back();
			auto brzero_idx = std::find_if(last.begin(), last.end(),
					[](const j5::instruction &i){return i.code == j5::op_t::BRZERO;});
			if (brzero_idx != last.end()) {
				brzero_idx->op = snippet.size() + 1;
			}
		}
		snippets.push_back(snippet);
	}
	return snippets;
}

prog_snippet index_on_stack(dcpu16::operand_t x);
prog_snippet address_on_stack(dcpu16::operand_t x);
prog_snippet value_on_stack(dcpu16::operand_t x);

#endif /* REGISTER_CONVERT_HPP */
