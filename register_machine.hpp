#ifndef REGISTER_MACHINE_HPP
#define REGISTER_MACHINE_HPP

#include <array>
#include <boost/variant.hpp>
#include <map>
#include <string>
#include <vector>

namespace dcpu16 {

enum class op_t {
	SET,
	ADD,
	SUB,
	MUL,
	MLI,
	DIV,
	DVI,
	MOD,
	MDI,
	AND,
	BOR,
	XOR,
	SHR,
	ASR,
	SHL,

	IFB,
	IFC,
	IFE,
	IFN,
	IFG,
	IFA,
	IFL,
	IFU,

	ADX,
	SBX,

	STI,
	STD,

	JSR,

	DAT,
	OUT,
	NUM_OPS,
};

static const std::array<std::string, (size_t)op_t::NUM_OPS> OP_T_STR{{
	"SET",
	"ADD",
	"SUB",
	"MUL",
	"MLI",
	"DIV",
	"DVI",
	"MOD",
	"MDI",
	"AND",
	"BOR",
	"XOR",
	"SHR",
	"ASR",
	"SHL",

	"IFB",
	"IFC",
	"IFE",
	"IFN",
	"IFG",
	"IFA",
	"IFL",
	"IFU",

	"ADX",
	"SBX",

	"STI",
	"STD",

	"JSR",

	"DAT",
	"OUT",
}};

enum class reg_t : uint8_t {
	A,
	B,
	C,
	X,
	Y,
	Z,
	I,
	J,
	PC,
	SP,
	EX,
	IA,
	NUM_REGS,
	BEGIN = A,
};

static const std::array<std::string, (size_t)reg_t::NUM_REGS> REG_T_STR{{
	"A", "B", "C", "X", "Y", "Z", "I", "J", "PC", "SP", "EX", "IA"
}};

reg_t find_reg(const std::string &r);
std::ostream& operator<<(std::ostream &os, reg_t r);


using operand_t = boost::variant<std::string, reg_t, uint16_t>;

struct instruction {
	op_t code;
	operand_t b, a;
	std::string label;
};

std::ostream& operator<<(std::ostream& os, const instruction& ins);
bool operator==(const instruction &a, const instruction &b);
bool operator!=(const instruction &a, const instruction &b);

using program = std::vector<instruction>;

program tokenise_source(const std::string &source);

class machine {
public:
	void run(const program &prog, bool verbose);
	std::string register_dump();

	uint16_t set_op(uint16_t b, uint16_t a);
	uint16_t add_op(uint16_t b, uint16_t a);
	uint16_t sub_op(uint16_t b, uint16_t a);
	uint16_t mul_op(uint16_t b, uint16_t a);
	uint16_t mli_op(uint16_t b, uint16_t a);
	uint16_t div_op(uint16_t b, uint16_t a);
	uint16_t dvi_op(uint16_t b, uint16_t a);
	uint16_t mod_op(uint16_t b, uint16_t a);
	uint16_t mdi_op(uint16_t b, uint16_t a);
	uint16_t and_op(uint16_t b, uint16_t a);
	uint16_t bor_op(uint16_t b, uint16_t a);
	uint16_t xor_op(uint16_t b, uint16_t a);
	uint16_t shr_op(uint16_t b, uint16_t a);
	uint16_t asr_op(uint16_t b, uint16_t a);
	uint16_t shl_op(uint16_t b, uint16_t a);
	uint16_t adx_op(uint16_t b, uint16_t a);
	uint16_t sbx_op(uint16_t b, uint16_t a);

private:
	std::array<uint16_t, (size_t)reg_t::NUM_REGS> regs; // Could be a map?

	std::array<uint16_t, 0x10000> mem;
	program cur_prog;
	bool terminate;
	bool skip_next;

	uint16_t find_label(const std::string &l);
	uint16_t get_val(const operand_t &x);
	void set_val(const operand_t &x, uint16_t val);
	inline uint16_t get_reg(reg_t r)
	{
		return this->regs.at(static_cast<size_t>(r));
	}
	inline void set_reg(reg_t r, uint16_t v)
	{
		this->regs.at(static_cast<size_t>(r)) = v;
	}

	void dat_func(operand_t x);
	void out_func(operand_t x);
};

using binop_func_t = std::function<uint16_t(machine*, uint16_t, uint16_t)>;
using binop_map = std::map<op_t, binop_func_t>;

static const binop_map BIN_OPS {{
	{op_t::SET, &machine::set_op},
	{op_t::ADD, &machine::add_op},
	{op_t::SUB, &machine::sub_op},
	{op_t::MUL, &machine::mul_op},
	{op_t::MLI, &machine::mli_op},
	{op_t::DIV, &machine::div_op},
	{op_t::DVI, &machine::dvi_op},
	{op_t::MOD, &machine::mod_op},
	{op_t::MDI, &machine::mdi_op},
	{op_t::AND, &machine::and_op},
	{op_t::BOR, &machine::bor_op},
	{op_t::XOR, &machine::xor_op},
	{op_t::SHR, &machine::shr_op},
	{op_t::ASR, &machine::asr_op},
	{op_t::SHL, &machine::shl_op},
	{op_t::ADX, &machine::adx_op},
	{op_t::SBX, &machine::sbx_op},
}};

using condop_func_t = std::function<bool(uint16_t, uint16_t)>;
using condop_map = std::map<op_t, condop_func_t>;

static const condop_map COND_OPS = {{
	{op_t::IFB, [](uint16_t b, uint16_t a){return (b & a) != 0;}},
	{op_t::IFC, [](uint16_t b, uint16_t a){return (b & a) == 0;}},
	{op_t::IFE, [](uint16_t b, uint16_t a){return b == a;}},
	{op_t::IFN, [](uint16_t b, uint16_t a){return b != a;}},
	{op_t::IFG, [](uint16_t b, uint16_t a){return b > a;}},
	{op_t::IFA, [](uint16_t b, uint16_t a){return static_cast<int16_t>(b) > static_cast<int16_t>(a);}},
	{op_t::IFL, [](uint16_t b, uint16_t a){return b < a;}},
	{op_t::IFU, [](uint16_t b, uint16_t a){return static_cast<int16_t>(b) < static_cast<int16_t>(a);}},
}};

}

#endif /* REGISTER_MACHINE_HPP */
