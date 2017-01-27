#include "interpreter.hpp"

#include <algorithm>
#include <boost/variant.hpp>
#include <sstream>
#include <vector>

uint8_t& z80regblock::operator[](const char r) {
	auto pos = z80regblock::ACTUAL_REGS.find(r);
	if (pos == std::string::npos) throw "Invalid register requested";
	return this->block[pos];
}

uint16_t& z80regblock::operator[](const char *r) {
	if (strlen(r) != 2) throw "Invalid combined register requested";
	auto pos1 = z80regblock::ACTUAL_REGS.find(r[0]);
	auto pos2 = z80regblock::ACTUAL_REGS.find(r[1]);
	if (pos1 == std::string::npos || pos2 == std::string::npos) throw "Invalid register requested";
	if (pos2 - pos1 != 1) throw "Invalid register combination";

	// Ew. Combines 2 uint8_ts into the relevant uint16_t register
	return *reinterpret_cast<uint16_t *>(&this->block[pos1]);
}

/* static */ const std::string z80regblock::ACTUAL_REGS = "afbcdehl";

z80instruction tokeniseLine(const std::string &line)
{
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

	z80instruction ins;

	auto it = words.begin();
	if (it->back() == ':') {
		ins.label = words.front();
		ins.label.pop_back();
		++it;
	}

	// Get operation
	auto opit = std::find(op_t_str.begin(), op_t_str.end(), *it);
	if (opit == op_t_str.end()) {
		throw "Unknown op code: " + *it;
	}
	ins.op = static_cast<op_t>(std::distance(op_t_str.begin(), opit));
	it++;

	if (it != words.end()) {
		ins.operand1 = *it;
		it++;
	}

	if (it != words.end()) {
		ins.operand2 = *it;
		it++;
	}
	return ins;
}

z80prog tokeniseSource(const std::string &source)
{
	std::stringstream iss(source);

	z80prog prog;
	std::string line;
	while (std::getline(iss, line)) {
		prog.push_back(tokeniseLine(line));
	}
	return prog;
}

void z80machine::nop_func()
{
	// Easy.
}
void z80machine::ret_func()
{

}
void z80machine::end_func()
{
	// Nothing to do.
}

void z80machine::inc_func(const std::string &x)
{
	if (x == "f") throw "Invalid register";

	if (x.size() == 1) {
		this->main[x[0]] += 1;
	} else if (x == "sp") {
		this->sp += 1;
	} else if (x == "(hl)") {
		this->mem[this->main["hl"]] += 1;
	} else {
		this->main[x.c_str()] += 1;
	}
}
void z80machine::dec_func(const std::string &x)
{
	if (x == "f") throw "Invalid register";

	if (x.size() == 1) {
		this->main[x[0]] -= 1;
	} else if (x == "sp") {
		this->sp -= 1;
	} else if (x == "(hl)") {
		this->mem[this->main["hl"]] -= 1;
	} else {
		this->main[x.c_str()] -= 1;
	}
}
void z80machine::and_func(const std::string &x)
{
}
void z80machine::or_func(const std::string &x)
{
}
void z80machine::xor_func(const std::string &x)
{
}
void z80machine::cp_func(const std::string &x)
{
}
void z80machine::sub_func(const std::string &x)
{
}
void z80machine::org_func(const std::string &x)
{
	// Ignored
	(void)x;
}

void z80machine::ld_func(const std::string &x, const std::string &y)
{
	boost::variant<uint8_t, uint16_t> r;
	if (x.size() == 1) {
		r = this->main[x[0]];
	} else if (x == "sp") {
		this->sp -= 1;
	} else if (x == "(hl)") {
		this->mem[this->main["hl"]] -= 1;
	} else {
		this->main[x.c_str()] -= 1;
	}
}
void z80machine::add_func(const std::string &x, const std::string &y)
{
}
void z80machine::adc_func(const std::string &x, const std::string &y)
{
}
void z80machine::sbc_func(const std::string &x, const std::string &y)
{
}
void z80machine::jp_func(const std::string &x, const std::string &y)
{
}
void z80machine::jr_func(const std::string &x, const std::string &y)
{
}
void z80machine::call_func(const std::string &x, const std::string &y)
{
}
