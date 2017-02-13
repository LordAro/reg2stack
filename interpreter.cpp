#include "interpreter.hpp"

#include <algorithm>
#include <chrono>
#include <boost/variant.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

/**
 * Register look up. Single character for the single register
 * @param r Register to lookup.
 * @return A reference to the register in memory
 */
uint8_t& z80regblock::operator[](const char r) {
	auto pos = z80regblock::ACTUAL_REGS.find(r);
	if (pos == std::string::npos) {
		throw "Invalid register '" + std::string(1, r) + "' requested";
	}
	return this->block[pos];
}

/**
 * Register look up. Multiple characters for the combined registers.
 * @param r Combined register to lookup.
 * @return A reference to the register in memory, horribly hacked together.
 */
uint16_t& z80regblock::operator[](const char *r) {
	assert(strlen(r) == 2);
	if (!z80regblock::is_reg(r)) {
		throw "Invalid register '" + std::string(r)  + "' requested";
	}

	auto pos = z80regblock::ACTUAL_REGS.find(r[0]);
	// Ew. Combines 2 uint8_ts into the relevant uint16_t register
	return *reinterpret_cast<uint16_t *>(&this->block[pos]);
}

/**
 * Gets whether a character is a valid register.
 * @param r Character to check.
 * @return If r is in the list of registers.
 */
/* static */ bool z80regblock::is_reg(char r)
{
	return z80regblock::ACTUAL_REGS.find(r) != std::string::npos;
}

/**
 * Like the single character is_reg, but with some addition checks to make sure
 * the two characters are valid combined registers.
 * @param r String to check.
 * @return If r is a valid combined register.
 */
/* static */ bool z80regblock::is_reg(const char *r)
{
	assert(strlen(r) == 2);
	auto pos1 = z80regblock::ACTUAL_REGS.find(r[0]);
	auto pos2 = z80regblock::ACTUAL_REGS.find(r[1]);
	if (pos1 == std::string::npos || pos2 == std::string::npos) return false;
	if (pos2 - pos1 != 1 || pos1 % 2 != 0) return false;
	return true;
}

/* static */ const std::string z80regblock::ACTUAL_REGS = "AFBCDEHL";

/**
 * Takes a line of input and tokenises it into something approximating
 * a z80 instruction. May or may not be a valid instruction.
 * @param line Input string.
 * @return The parsed instruction.
 */
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
		for (auto &c : ins.operand1) c = toupper(c);
		it++;
	}

	if (it != words.end()) {
		ins.operand2 = *it;
		for (auto &c : ins.operand2) c = toupper(c);
		it++;
	}
	return ins;
}

/**
 * Stream operator for printing an instruction to console
 * @param os The stream.
 * @param ins The instruction.
 * @return The modified stream.
 */
std::ostream& operator<<(std::ostream& os, const z80instruction& ins)
{
	os << '{';
	if (!ins.label.empty()) os << ins.label << ": ";
	os << op_t_str.at((size_t)ins.op);
	if (!ins.operand1.empty()) os << ' ' << ins.operand1;
	if (!ins.operand2.empty()) os << ' ' << ins.operand2;
	os << '}';
	return os;
}

/**
 * Tokenises a source file string.
 * @param source The source (as a complete string).
 * @return The whole parsed program.
 */
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

/**
 * Runs a program on a z80 machine! Runs at a particular (currently hardcoded) clock-rate.
 * @param prog The (parsed) program to run
 */
void z80machine::run(const z80prog &prog)
{
	this->cur_prog = prog;
	for (; this->pc < this->cur_prog.size(); this->pc++) {
		auto start = std::chrono::high_resolution_clock::now();

		std::cout << this->cur_prog[this->pc] << '\n';

		const auto &ins = this->cur_prog[this->pc];
		auto func = OPERATIONS[(size_t)ins.op];
		if (func.which() == 0) {
			boost::get<op0func_t>(func)(this);
		} else if (func.which() == 1) {
			boost::get<op1func_t>(func)(this, ins.operand1);
		} else {
			boost::get<op2func_t>(func)(this, ins.operand1, ins.operand2);
		}

		std::cout << this->register_dump();

		std::this_thread::sleep_until(start + std::chrono::milliseconds(10)); // arbitrary
	}
}

/**
 * Dumps the current state of the machine's registers into a string, formatted as hex numbers.
 * @return The registers in string form.
 */
std::string z80machine::register_dump()
{
	std::stringstream ss;
	ss << std::hex;
	ss << "PC: 0x" << this->pc << '\n';
	ss << "SP: 0x" << this->sp << '\n';
	ss << "Regs: 0x" << (size_t)this->main['A'] << " 0x" << (size_t)this->main['F'] << '\n';
	ss << "      0x" << (size_t)this->main['B'] << " 0x" << (size_t)this->main['C'] << '\n';
	ss << "      0x" << (size_t)this->main['D'] << " 0x" << (size_t)this->main['E'] << '\n';
	ss << "      0x" << (size_t)this->main['H'] << " 0x" << (size_t)this->main['L'] << '\n';
	return ss.str();
}

bool is_hex(const std::string &n)
{
	return std::all_of(n.begin(), n.end(), ::isxdigit);
}

bool is_bit_set(uint8_t n, uint8_t b)
{
	return ((n >> b) & 1) == 1;
}

boost::variant<std::string, uint8_t, uint16_t> z80machine::get_val(const std::string &val)
{
	// wish i had pattern matching
	if (val.back() == 'H' && is_hex(val.substr(0, val.size() - 1))) {
		// Hex literal
		return static_cast<uint16_t>(stoul(val, nullptr, 16));
	} else if (val.size() == 1 && z80regblock::is_reg(val[0])) {
		// std register
		return this->main[val[0]];
	} else if (val.size() == 2 && z80regblock::is_reg(val.c_str())) {
		// combo register
		return this->main[val.c_str()];
	} else if (val == "SP") {
		return this->sp;
	} else if (val.front() == '(' && val.back() == ')') {
		std::string in = val.substr(1, val.size() - 2);
		if (in.back() == 'H' && is_hex(in.substr(0, in.size() - 1))) {
			return this->mem[stoul(in, nullptr, 16)];
		}
		throw "Tried to get value from unknown: " + val;
	}
	return val;
}

uint16_t z80machine::find_label(const std::string &l)
{
	auto pos = std::find_if(this->cur_prog.begin(), this->cur_prog.end(), [l](const z80instruction &i) { return i.label == l; });
	if (pos == this->cur_prog.end()) {
		throw "Undefined label " + l + "used";
	}
	return std::distance(this->cur_prog.begin(), pos);
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
	if (x == "F") throw "Invalid register";

	if (x.size() == 1 && z80regblock::is_reg(x[0])) {
		this->main[x[0]] += 1;
		if (this->main[x[0]] == 0) {
			this->main['F'] |= 1 << z80regblock::Zero;
		}
	} else if (x.size() == 2 && z80regblock::is_reg(x.c_str())) {
		this->main[x.c_str()] += 1;
	} else if (x == "SP") {
		this->sp += 1;
	} else if (x == "(HL)") {
		this->mem[this->main["HL"]] += 1;
	} else {
		throw "inc wut";
	}
}

/**
 */
void z80machine::dec_func(const std::string &x)
{
	if (x == "F") throw "Invalid register";

	// TODO Work out something better than this. Please.
	if (x.size() == 1 && z80regblock::is_reg(x[0])) {
		this->main[x[0]] -= 1;
		if (this->main[x[0]] == 0) { // TODO Unfinished flags
			this->main['F'] |= 1 << z80regblock::Zero;
		}
	} else if (x.size() == 2 && z80regblock::is_reg(x.c_str())) {
		this->main[x.c_str()] -= 1;
	} else if (x == "SP") {
		this->sp -= 1;
	} else if (x == "(HL)") {
		this->mem[this->main["HL"]] -= 1;
	} else {
		throw "dec wut";
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
	// Ignored?
	(void)x;
}

void z80machine::ld_func(const std::string &x, const std::string &y)
{
	auto v = this->get_val(y);

	if (x.size() == 1 && z80regblock::is_reg(x[0])) {
		this->main[x[0]] = (uint8_t)boost::get<uint16_t>(v);
	} else if (x.size() == 2 && z80regblock::is_reg(x.c_str())) {
		this->main[x.c_str()] = boost::get<uint16_t>(v);
	} else if (x == "SP") {
		this->sp = boost::get<uint16_t>(v);
	} else if (x == "(HL)") {
		this->mem[this->main["HL"]] = boost::get<uint8_t>(v);
	} else {
		throw "??";
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
	std::string param = y;
	if (y.size() == 0) {
		param = x;
	} else if (x == "NZ" || x == "Z") {
		if (is_bit_set(this->main['F'], z80regblock::Zero) == (x != "Z")) {
			return;
		}
	//} else if (x == "NC" || x == "C") {
	//} else if (x == "PO") {
	//} else if (x == "PE") {
	//} else if (x == "P") {
	//} else if (x == "M") {
	} else {
		throw "Unimplemented instruction JP " + x;
	}
	// hex literal or label
	uint16_t new_pos;
	if (param.back() == 'H' && is_hex(param.substr(0, param.size() - 1))) {
		new_pos = static_cast<uint16_t>(stoul(param, nullptr, 16));
	} else {
		new_pos = this->find_label(param);
	}
	this->pc = new_pos - 1; // auto increment
}
void z80machine::jr_func(const std::string &x, const std::string &y)
{
}
void z80machine::call_func(const std::string &x, const std::string &y)
{
	// 1 parameter version
	if (y.size() == 0) {
		uint16_t new_pos = this->find_label(x);
		this->pc = new_pos - 1; // gets incremented automatically
	} else {
		throw "conditional CALL unimplemented";
	}
}
