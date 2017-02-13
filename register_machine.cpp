#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "register_machine.hpp"

namespace dcpu16 {


bool is_hex(const std::string &s)
{
	return std::all_of(s.begin(), s.end(), ::isxdigit);
}

reg_t find_reg(const std::string &r)
{
	auto pos = std::find(REG_T_STR.begin(), REG_T_STR.end(), r);
	if (pos == REG_T_STR.end()) {
		throw "Not a register: " + r;
	}

	return static_cast<reg_t>(pos - REG_T_STR.begin());
}

std::ostream& operator<<(std::ostream &os, reg_t r)
{
	os << REG_T_STR.at(static_cast<size_t>(r));
	return os;
}

/**
 * Stream operator for printing an instruction to console
 * @param os The stream.
 * @param ins The instruction.
 * @return The modified stream.
 */
std::ostream& operator<<(std::ostream& os, const instruction& ins)
{
	if (!ins.label.empty()) os << ins.label << ": ";
	os << OP_T_STR.at((size_t)ins.code);
	os << " '" << ins.b << "'";
	if (!(ins.a.which() == 0 && boost::get<std::string>(ins.a).empty())) os << " '" << ins.a << "'";
	return os;
}

bool operator==(const instruction &a, const instruction &b)
{
	return a.code == b.code && a.b == b.b && a.a == b.a;
}

bool operator!=(const instruction &a, const instruction &b)
{
	return !(a == b);
}

const instruction instruction::NONE = {op_t::NUM_OPS, 0, 0, ""};


operand_t get_operand(const std::string &tok)
{
	operand_t ret;
	if (tok.size() > 2 && tok[0] == '0'
			&& tolower(tok[1]) == 'x' && is_hex(tok.substr(2))) {
		// hex literal
		ret = static_cast<uint16_t>(std::stoul(tok.substr(2), nullptr, 16));
	} else if (std::all_of(tok.begin(), tok.end(), ::isdigit)) {
		// decimal literal
		ret = static_cast<uint16_t>(std::stoul(tok));
	} else if (std::find(REG_T_STR.begin(), REG_T_STR.end(), tok) != REG_T_STR.end()) {
		// register (& pc, sp, etc)
		ret = find_reg(tok);
	} else if (tok.size() >= 2 && tok.front() == '"' && tok.back() == '"') {
		ret = tok.substr(1, tok.length() - 2); // TODO: unescape control chars
	} else {
		// probably a string literal or label.
		ret = tok;
	}
	return ret;
}

instruction tokenise_line(const std::string &line)
{
	// Parse into separate words
	std::vector<std::string> words;
	bool inword = false;
	bool inquote = false;
	for (char c : line) {
		if (c == ';') break; // comment
		if (c == '"') inquote = !inquote;
		if (!inquote && (isspace(c) || c == ',')) {
			inword = false;
			continue;
		} else if (!inword) {
			words.emplace_back(); // start next word
		}
		words.back().push_back(c);
		inword = true;
	}

	if (words.empty()) return instruction::NONE;

	instruction ins;

	auto it = words.begin();
	if (it->front() == ':') {
		ins.label = it->substr(1);
		++it;
	}

	// Get operation
	auto opit = std::find(OP_T_STR.begin(), OP_T_STR.end(), *it);
	if (opit == OP_T_STR.end()) {
		throw "Unknown op code: " + *it;
	}
	ins.code = static_cast<op_t>(std::distance(OP_T_STR.begin(), opit));
	it++;

	// All instructions have at least one operand
	ins.b = get_operand(*it++);

	if (it != words.end()) {
		ins.a = get_operand(*it++);
	} else if (ins.code != op_t::OUT && ins.code != op_t::DAT) {
		throw "Incorrect number of operands for " + OP_T_STR.at((size_t)ins.code); // OUT & DAT have one operand
	}
	return ins;

}

program tokenise_source(const std::string &source)
{
	std::stringstream iss(source);

	program prog;
	std::string line;

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = source.find('\n', prev)) != std::string::npos)
	{
		std::string line = source.substr(prev, pos - prev);
		instruction ins = tokenise_line(line);
		if (ins != instruction::NONE) prog.push_back(ins);
		prev = pos + 1;
	}
	return prog;
}

uint16_t machine::find_label(const std::string &l)
{
	auto pos = std::find_if(this->cur_prog.begin(), this->cur_prog.end(), [l](const instruction &i) { return i.label == l; });
	if (pos == this->cur_prog.end()) {
		throw "Undefined label '" + l + "' used";
	}
	return std::distance(this->cur_prog.begin(), pos);
}

uint16_t machine::get_val(const operand_t &x)
{
	switch (x.which()) {
		case 0: { // string
			// much sad here. duplicates a lot of get_operand
			std::string op = boost::get<std::string>(x);
			if (op.find('+') != std::string::npos) { // expression. needs expanding to others
				size_t pos = op.find('+');
				return this->get_val(op.substr(0, pos)) + this->get_val(op.substr(pos + 1));
			} else if (op.size() > 2 && op.front() == '[' && op.back() == ']') { // array val
				return this->mem.at(this->get_val(op.substr(1, op.length() - 2)));
			} else if (op.size() > 2 && op[0] == '0' && tolower(op[1]) == 'x' && is_hex(op.substr(2))) { // hex literal
				return static_cast<uint16_t>(std::stoul(op.substr(2), nullptr, 16));
			} else if (std::find(REG_T_STR.begin(), REG_T_STR.end(), op) != REG_T_STR.end()) { // reg str
				return this->regs.at(static_cast<size_t>(find_reg(op)));
			}
			return this->find_label(op);
		}
		case 1: // reg
			return this->regs.at(static_cast<size_t>(boost::get<reg_t>(x)));
		case 2: // literal
			return boost::get<uint16_t>(x);
	}
	throw "Could not get value??";
}

void machine::run(const program &prog, bool stack = false, bool verbose = false)
{
	this->cur_prog = prog;
	this->terminate = false;
	this->skip_next = false;
	this->regs[(size_t)reg_t::SP] = 0xffff;


	uint16_t &pc = this->regs[(size_t)reg_t::PC];
	for (; !this->terminate && pc < this->cur_prog.size(); pc++) {
		auto start = std::chrono::high_resolution_clock::now();

		if (skip_next) {
			skip_next = false;
			continue;
		}


		const auto &ins = this->cur_prog[pc];
		std::cerr << ins << '\n';
		switch (ins.code) {
			case op_t::OUT:
				this->out_func(ins.b);
				break;
			case op_t::DAT:
				this->dat_func(ins.b);
				break;
			default:
				try {
					auto func = OPERATIONS.at((size_t)ins.code);
					func(this, ins.b, ins.a);
				} catch (std::bad_function_call) {
					throw "Unrecognised instruction " + OP_T_STR.at((size_t)ins.code);
				}
				break;
		}
		if (verbose) std::cout << this->register_dump() << '\n';
		std::this_thread::sleep_until(start + std::chrono::milliseconds(100)); // arbitrary
	}
}


template<typename ... Args>
std::string string_format(const std::string& format, Args... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

std::string machine::register_dump()
{
	uint16_t pc = this->regs[(size_t)reg_t::PC];
	uint16_t sp = this->regs[(size_t)reg_t::SP];
	uint16_t ia = this->regs[(size_t)reg_t::IA];
	uint16_t ex = this->regs[(size_t)reg_t::EX];

	uint16_t a = this->regs[(size_t)reg_t::A];
	uint16_t b = this->regs[(size_t)reg_t::B];
	uint16_t c = this->regs[(size_t)reg_t::C];
	uint16_t x = this->regs[(size_t)reg_t::X];
	uint16_t y = this->regs[(size_t)reg_t::Y];
	uint16_t z = this->regs[(size_t)reg_t::Z];
	uint16_t i = this->regs[(size_t)reg_t::I];
	uint16_t j = this->regs[(size_t)reg_t::J];
	return string_format("PC %04x SP %04x IA %04x EX %04x\n"
	                     "A  %04x B  %04x C  %04x\n"
	                     "X  %04x Y  %04x Z  %04x\n"
	                     "I  %04x J  %04x",
	                     pc, sp, ia, ex,
	                     a, b, c, x, y, z, i, j);

}


void machine::set_func(operand_t x, operand_t y)
{
	uint16_t val = this->get_val(y);
	switch (x.which()) {
		case 0: {
			std::string op = boost::get<std::string>(x);
			if (op.size() > 2 && op.front() == '[' && op.back() == ']') {
				this->mem.at(this->get_val(op.substr(1, op.length() - 2))) = val;
			}
			break;
		}
		case 1:
			this->regs[(size_t)boost::get<reg_t>(x)] = val;
			break;
	}
}

void machine::add_func(operand_t x, operand_t y)
{
	uint16_t y_val = this->get_val(y);
	switch (x.which()) {
		case 0: {
			std::string op = boost::get<std::string>(x);
			if (op.size() > 2 && op.front() == '[' && op.back() == ']') {
				this->mem.at(this->get_val(op.substr(1, op.length() - 2))) += y_val;
			}
			break;
		}
		case 1:
			this->regs[(size_t)boost::get<reg_t>(x)] += y_val;
			break;
	}
	// TODO: Set EX
}

void machine::sub_func(operand_t x, operand_t y)
{
	uint16_t y_val = this->get_val(y);
	switch (x.which()) {
		case 0: {
			std::string op = boost::get<std::string>(x);
			if (op.size() > 2 && op.front() == '[' && op.back() == ']') {
				this->mem.at(this->get_val(op.substr(1, op.length() - 2))) -= y_val;
			}
			break;
		}
		case 1:
			this->regs[(size_t)boost::get<reg_t>(x)] -= y_val;
			break;
	}
	// TODO: Set EX
}

void machine::mul_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction MUL";
}

void machine::mli_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction MLI";
}

void machine::div_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction DIV";
}

void machine::dvi_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction DVI";
}

void machine::mod_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction MOD";
}

void machine::mdi_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction MDI";
}

void machine::and_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction AND";
}

void machine::bor_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction BOR";
}

void machine::xor_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction XOR";
}

void machine::shr_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction SHR";
}

void machine::asr_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction ASR";
}

void machine::shl_func(operand_t x, operand_t y)
{
	uint16_t y_val = this->get_val(y);
	switch (x.which()) {
		case 0: {
			std::string op = boost::get<std::string>(x);
			if (op.size() > 2 && op.front() == '[' && op.back() == ']') {
				this->mem.at(this->get_val(op.substr(1, op.length() - 2))) <<= y_val;
			}
			break;
		}
		case 1:
			this->regs[(size_t)boost::get<reg_t>(x)] <<= y_val;
			break;
	}
	//TODO: set EX to ((b<<a)>>16)&0xffff
}


void machine::ifb_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFB";
}

void machine::ifc_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFC";
}

void machine::ife_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFE";
}

void machine::ifn_func(operand_t x, operand_t y)
{
	uint16_t x_val = this->get_val(x);
	uint16_t y_val = this->get_val(y);
	if (!(x_val != y_val)) this->skip_next = true;
}

void machine::ifg_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFG";
}

void machine::ifa_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFA";
}

void machine::ifl_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFL";
}

void machine::ifu_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction IFU";
}


void machine::adx_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction ADX";
}

void machine::sbx_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction SBX";
}


void machine::sti_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction STI";
}

void machine::std_func(operand_t x, operand_t y)
{
	throw "Unimplemented instruction STD";
}

void machine::dat_func(operand_t x)
{
	if (x.which() == 2 && boost::get<uint16_t>(x) == 0) {
		this->terminate = true;
	}
}

void machine::out_func(operand_t x)
{
	uint16_t addr = this->get_val(x);
	switch (x.which()) {
		case 0:
			if (isalpha(boost::get<std::string>(x)[0])) { // points to a label
				std::cout << this->cur_prog.at(addr).b;
			} else {
				std::cout << std::to_string(addr);
			}
			break;
		case 1:
			std::cout << std::to_string(addr);
			break;
		case 2:
			std::cout << this->cur_prog.at(addr);
			break;
	}
}

}
