#include <boost/optional.hpp>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "stack_machine.hpp"
#include "util.hpp"

namespace j5 {

std::ostream& operator<<(std::ostream& os, const instruction& ins)
{
	if (!ins.label.empty()) os << ins.label << ": ";
	os << OP_T_STR.at((size_t)ins.code);
	if (ins.op != boost::none) os << ' ' << *ins.op;
	return os;
}

operand_t get_operand(const std::string &tok)
{
	operand_t ret;
	if (tok.size() > 1 && tolower(tok.back()) == 'h'
			&& is_hex(tok.substr(0, tok.size() - 1))) {
		// hex literal
		ret = static_cast<uint16_t>(std::stoul(tok.substr(0, tok.size() - 1), nullptr, 16));
	} else if (std::all_of(tok.begin(), tok.end(), ::isdigit)) {
		// decimal literal
		ret = static_cast<uint16_t>(std::stoul(tok));
	} else {
		throw "Could not get operand from " + tok;
	}
	return ret;
}

boost::optional<instruction> tokenise_line(const std::string &line)
{
	auto words = split_words(line);
	if (words.empty()) return boost::none;

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

	// Only SET & OUT have 1 operand
	if (it != words.end() && ins.code == op_t::SET) {
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

void machine::run(const program &prog, bool verbose)
{
	this->cur_prog = prog;
	this->terminate = false;

	for (; !this->terminate && this->pc < this->cur_prog.size(); this->pc++) {
		auto start = std::chrono::high_resolution_clock::now();

		const auto &ins = this->cur_prog[pc];
		std::cerr << ins << '\n';
		switch (ins.code) {
			case op_t::SET:
				this->set_func(*ins.op);
				break;
			default:
				try {
					auto func = OPERATIONS.at(ins.code);
					func(this);
				} catch (std::bad_function_call) {
					throw "Unrecognised instruction " + OP_T_STR.at((size_t)ins.code);
				}
		}
		if (verbose) std::cout << this->register_dump() << '\n';
		std::this_thread::sleep_until(start + std::chrono::milliseconds(100)); // arbitrary
	}
}

std::string machine::register_dump()
{
	std::string ret = string_format("PC %04x\t", this->pc);
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
	{op_t::NOP,    [](machine*){}},
	{op_t::DROP,   [](machine* m){m->stack.pop();}},
	{op_t::LOAD,   &machine::load_func},
	{op_t::STORE,  &machine::store_func},
	{op_t::DUP,    [](machine* m){m->stack.push(m->stack.top());}},
	{op_t::SWAP,   &machine::swap_func},
	{op_t::INC,    [](machine *m){m->stack.top()++;}},
	{op_t::DEC,    [](machine *m){m->stack.top()--;}},
	{op_t::ADD,    [](machine *m){m->binop_func(std::plus<>());}},
	{op_t::SUB,    [](machine *m){m->binop_func(std::minus<>());}},
//	{op_t::ADDC,   [](machine *m){m->terminate = true;}},
//	{op_t::SUBC,   [](machine *m){m->terminate = true;}},
	{op_t::AND,    [](machine *m){m->binop_func(std::bit_and<>());}},
	{op_t::OR,     [](machine *m){m->binop_func(std::bit_or<>());}},
	{op_t::XOR,    [](machine *m){m->binop_func(std::bit_xor<>());}},
	{op_t::NOT,    [](machine *m){m->stack.top() = ~m->stack.top();}},
	{op_t::SHR,    [](machine *m){m->binop_func([](uint16_t a, uint16_t b){return a >> b;});}},
	{op_t::SHL,    [](machine *m){m->binop_func([](uint16_t a, uint16_t b){return a << b;});}},
	{op_t::TEQ,    [](machine *m){m->terminate = true;}},
	{op_t::TGT,    [](machine *m){m->terminate = true;}},
	{op_t::BRZERO, [](machine *m){m->terminate = true;}},
	{op_t::STOP,   [](machine *m){m->terminate = true;}},
	{op_t::OUT,    [](machine *m){std::cout << m->stack.top() << '\n';}},
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

}
