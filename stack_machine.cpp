#include <boost/optional.hpp>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "stack_machine.hpp"
#include "register_convert.hpp"
#include "util.hpp"

namespace j5 {

std::ostream& operator<<(std::ostream& os, const instruction& ins)
{
	if (!ins.label.empty()) os << ins.label << ": ";
	os << OP_T_STR.at((size_t)ins.code);
	if (ins.op.which() != 0) os << ' ' << ins.op;
	return os;
}

operand_t get_operand(const std::string &tok)
{
	operand_t ret = boost::blank();
	if (tok.size() > 1 && tolower(tok.back()) == 'h'
			&& is_hex(tok.substr(0, tok.size() - 1))) {
		// hex literal
		ret = static_cast<uint16_t>(std::stoul(tok.substr(0, tok.size() - 1), nullptr, 16));
	} else if (std::all_of(tok.begin(), tok.end(), ::isdigit)) {
		// decimal literal
		ret = static_cast<uint16_t>(std::stoul(tok));
	} else {
		// assume label
		ret = tok;
	}
	return ret;
}

boost::optional<instruction> tokenise_line(const std::string &line)
{
	auto words = split_words(line);
	if (words.empty()) return boost::none;

	instruction ins;
	auto it = words.begin();
	if (it->back() == ':') { // LABEL:
		ins.label = it->substr(0, it->size() - 1);
		++it;
	}

	// Get operation
	auto opit = std::find(OP_T_STR.begin(), OP_T_STR.end(), *it);
	if (opit == OP_T_STR.end()) {
		throw "Unknown op code: " + *it;
	}
	ins.code = static_cast<op_t>(std::distance(OP_T_STR.begin(), opit));
	it++;

	if (it != words.end() && (ins.code == op_t::SET || ins.code == op_t::BRANCH || ins.code == op_t::BRZERO)) {
		ins.op = get_operand(*it++);
	}

	if (it != words.end()) {
		throw "Incorrect number of operands for " + OP_T_STR.at((size_t)ins.code);
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
		boost::optional<instruction> ins = tokenise_line(line);
		if (ins) prog.push_back(*ins);
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

uint16_t machine::run_branch_instruction(const instruction &ins)
{
	switch (ins.code) {
		case op_t::SET:
			if (ins.op.which() != 1) {
				throw "Tried to SET to a label";
			}
			this->set_func(boost::get<uint16_t>(ins.op));
			break;
		case op_t::BRZERO:
			if (!HasBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO))) {
				break;
			}
			ClrBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
			/* FALLTHROUGH */
		case op_t::BRANCH:
			if (ins.op.which() == 1) {
				return this->pc + boost::get<uint16_t>(ins.op); // relative if number
			} else {
				return this->find_label(boost::get<std::string>(ins.op));
			}
		default:
			throw "Not reached";
	}
	return this->pc + 1;
}

/**
 * Runs an instruction on the stack machine
 * @param ins Instruction to run
 * @return New value of program counter
 */
uint16_t machine::run_instruction(const instruction &ins)
{
	if (ins.code == op_t::SET || ins.code == op_t::BRANCH || ins.code == op_t::BRZERO) {
		if (ins.op.which() == 0) {
			throw "Missing operand for " + OP_T_STR.at((size_t)ins.code);
		}
		return this->run_branch_instruction(ins);
	}

	try {
		auto func = OPERATIONS.at(ins.code);
		func(this);
	} catch (std::out_of_range) {
		throw "Unrecognised instruction " + OP_T_STR.at((size_t)ins.code);
	}
	return this->pc + 1;
}

void machine::run(const program &prog, bool speedlimit)
{
	this->cur_prog = prog;
	this->terminate = false;

	while (!this->terminate && this->pc < this->cur_prog.size()) {
		auto start = std::chrono::high_resolution_clock::now();

		const auto &ins = this->cur_prog[pc];
		log<LOG_DEBUG>(ins);
		this->pc = this->run_instruction(ins);

		log<LOG_DEBUG2>(this->register_dump());
		if (speedlimit) {
			std::this_thread::sleep_until(start + std::chrono::milliseconds(100)); // arbitrary
		}
	}
}

std::string machine::register_dump()
{
	std::string ret = string_format("PC %04x\tFLAGS %04x\t", this->pc, this->flags);
	ret += '(';
	auto stack_copy = this->stack; // urgh.
	while (!stack_copy.empty()) {
		ret += string_format("%04x,", stack_copy.top());
		stack_copy.pop();
	}
	ret += ')';
	return ret;
}


/* static */ const machine::op_map machine::OPERATIONS {{
	{op_t::ADD, [](machine *m){m->binop_func(std::plus<>());}},
	{op_t::SUB, [](machine *m){m->binop_func(std::minus<>());}},
	{op_t::INC, [](machine *m){m->stack.top()++;}},
	{op_t::DEC, [](machine *m){m->stack.top()--;}},
	{op_t::AND, [](machine *m){m->binop_func(std::bit_and<>());}},
	{op_t::OR,  [](machine *m){m->binop_func(std::bit_or<>());}},
	{op_t::NOT, [](machine *m){m->stack.top() = ~m->stack.top();}},
	{op_t::XOR, [](machine *m){m->binop_func(std::bit_xor<>());}},
	{op_t::SHR, [](machine *m){m->binop_func([](uint16_t a, uint16_t b){return a >> b;});}},
	{op_t::SHL, [](machine *m){m->binop_func([](uint16_t a, uint16_t b){return a << b;});}},

	{op_t::TGT, [](machine *m){m->comp_func(std::greater<>());}},
	{op_t::TLT, [](machine *m){m->comp_func(std::less<>());}},
	{op_t::TEQ, [](machine *m){m->comp_func(std::equal_to<>());}},
	{op_t::TSZ, &machine::testzero_func},

	// SET special, SSET unimplemented
	{op_t::LOAD,   &machine::load_func},
	{op_t::STORE,  &machine::store_func},
	// BRANCH, BRZERO special
//	{op_t::IBRANCH, [](machine *m){m->terminate = true;}},
//	{op_t::CALL,    [](machine *m){m->terminate = true;}},
//	{op_t::RETURN,  [](machine *m){m->terminate = true;}},
	{op_t::STOP,    [](machine *m){m->terminate = true;}},
	{op_t::OUT,     [](machine *m){std::cout << m->stack.top() << '\n';}},

	{op_t::DROP,  [](machine* m){m->stack.pop();}},
	{op_t::DUP,   [](machine* m){m->stack.push(m->stack.top());}},
	{op_t::SWAP,  &machine::swap_func},
	{op_t::RSD3,  &machine::rsd3_func},
	{op_t::RSU3,  &machine::rsu3_func},
	{op_t::TUCK2, &machine::tuck2_func},
	{op_t::TUCK3, &machine::tuck3_func},
	{op_t::COPY3, &machine::copy3_func},
	// PUSH,POP special
}};

void machine::set_func(uint16_t v)
{
	this->stack.push(v);
}

void machine::load_func()
{
	uint16_t addr = this->stack.top();
	this->stack.pop();
	this->stack.push(this->mem.at(addr));
}

void machine::store_func()
{
	uint16_t addr = this->stack.top();
	this->stack.pop();
	uint16_t val = this->stack.top();
	this->stack.pop();
	this->mem.at(addr) = val;
}

void machine::swap_func()
{
	uint16_t a = this->stack.top();
	this->stack.pop();
	uint16_t b = this->stack.top();
	this->stack.pop();
	this->stack.push(a);
	this->stack.push(b);
}

void machine::binop_func(std::function<uint16_t(uint16_t, uint16_t)> op)
{
	uint16_t a = this->stack.top();
	this->stack.pop();
	uint16_t b = this->stack.top();
	this->stack.pop();
	this->stack.push(op(b, a));
}

void machine::comp_func(std::function<bool(uint16_t, uint16_t)> op)
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top(); // no peeking..
	this->stack.push(top); // restore
	// sets nth bit (zero) to result of op
	if (op(top, next)) {
		SetBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
	} else {
		ClrBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
	}
}

void machine::testzero_func()
{
	if (this->stack.top() == 0) {
		SetBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
	} else {
		ClrBit(this->flags, static_cast<uint8_t>(machine::flagbit::ZERO));
	}
}

void machine::tuck2_func()
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top();
	this->stack.pop();
	this->stack.push(top);
	this->stack.push(next);
	this->stack.push(top);
}

void machine::tuck3_func()
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top();
	this->stack.pop();
	uint16_t third = this->stack.top();
	this->stack.pop();
	this->stack.push(top);
	this->stack.push(third);
	this->stack.push(next);
	this->stack.push(top);
}

void machine::rsu3_func()
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top();
	this->stack.pop();
	uint16_t third = this->stack.top();
	this->stack.pop();
	this->stack.push(top);
	this->stack.push(third);
	this->stack.push(next);
}

void machine::rsd3_func()
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top();
	this->stack.pop();
	uint16_t third = this->stack.top();
	this->stack.pop();
	this->stack.push(next);
	this->stack.push(top);
	this->stack.push(third);
}

void machine::copy3_func()
{
	uint16_t top = this->stack.top();
	this->stack.pop();
	uint16_t next = this->stack.top();
	this->stack.pop();
	uint16_t third = this->stack.top();
	this->stack.push(next);
	this->stack.push(top);
	this->stack.push(third);
}
}
