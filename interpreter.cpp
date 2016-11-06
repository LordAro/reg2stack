#include "interpreter.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

enum Type {
	Illegal,
	Num,
	Opcode,
	Symbol,
	String,
};

void tokeniseLine(const std::string &line) {
	std::vector<std::string> words(1);
	bool inword = false;
	for (char c : line) {
		if (c == ';') break; // comment
		if (isspace(c) || c == ',') {
			if (inword) words.emplace_back(); // start next word
			inword = false;
			continue;
		}
		words.back().push_back(c);
		inword = true;
	}

	for (auto it = words.begin(); it != words.end(); it++) {
		std::string w = *it;
		if (it == words.begin() && w.back() == ':') { // label
			// TODO link label to line
			continue;
		}
		auto ins = std::find_if(ins_bases.begin(), ins_bases.end(), [w](const InsBase &is){ return is.str == w; });
		if (ins != ins_bases.end()) {
			if (ins->args != static_cast<size_t>(std::distance(it, words.end() - 1))) {
				std::cerr << "Invalid number of args for " << w << ", expected: " << ins->args << ", found: " << std::distance(it, words.end() - 1) << "\n";
			}
			switch (ins->args) {
				case 1: ins->func(*(it + 1)); break;
				case 2: ins->func(*(it + 1), *(it + 2)); break;
			}
		} else {
			std::cerr << "Unimplemented/unrecognised instruction! " << w << "\n";
		}
	}
	for (auto &w : words) std::cout << w << '$';
	std::cout << "\n";
}

z80prog tokeniseSource(const std::string &source)
{
	std::stringstream iss(source);
	std::string line;
	while (std::getline(iss, line)) {
		tokeniseLine(line);
	}
	return z80prog{};
}

