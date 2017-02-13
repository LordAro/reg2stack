#ifndef REGISTER_MACHINE_HPP
#define REGISTER_MACHINE_HPP

#include <array>
#include <cstdint>
#include <boost/variant.hpp>
#include <string>
#include <vector>

namespace dcpu16 {

bool is_hex(const std::string &s);

enum class op_t : size_t {
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

	static const instruction NONE;
};

std::ostream& operator<<(std::ostream& os, const instruction& ins);
bool operator==(const instruction &a, const instruction &b);
bool operator!=(const instruction &a, const instruction &b);

using program = std::vector<instruction>;

program tokenise_source(const std::string &source);

class machine {
public:
	std::array<uint16_t, (size_t)reg_t::NUM_REGS> regs; // Could be a map

	std::array<uint16_t, 0x10000> mem;
	program cur_prog;
	bool terminate;
	bool skip_next;

	void run(const program &prog, bool stack, bool verbose);
	uint16_t find_label(const std::string &l);
	uint16_t get_val(const operand_t &x);
	std::string register_dump();

	void set_func(operand_t x, operand_t y);
	void add_func(operand_t x, operand_t y);
	void sub_func(operand_t x, operand_t y);
	void mul_func(operand_t x, operand_t y);
	void mli_func(operand_t x, operand_t y);
	void div_func(operand_t x, operand_t y);
	void dvi_func(operand_t x, operand_t y);
	void mod_func(operand_t x, operand_t y);
	void mdi_func(operand_t x, operand_t y);
	void and_func(operand_t x, operand_t y);
	void bor_func(operand_t x, operand_t y);
	void xor_func(operand_t x, operand_t y);
	void shr_func(operand_t x, operand_t y);
	void asr_func(operand_t x, operand_t y);
	void shl_func(operand_t x, operand_t y);

	void ifb_func(operand_t x, operand_t y);
	void ifc_func(operand_t x, operand_t y);
	void ife_func(operand_t x, operand_t y);
	void ifn_func(operand_t x, operand_t y);
	void ifg_func(operand_t x, operand_t y);
	void ifa_func(operand_t x, operand_t y);
	void ifl_func(operand_t x, operand_t y);
	void ifu_func(operand_t x, operand_t y);

	void adx_func(operand_t x, operand_t y);
	void sbx_func(operand_t x, operand_t y);

	void sti_func(operand_t x, operand_t y);
	void std_func(operand_t x, operand_t y);

	void dat_func(operand_t x);
	void out_func(operand_t x);
};

using opfunc_t = std::function<void(machine*, operand_t, operand_t)>;
using operations_map = std::array<opfunc_t, (size_t)op_t::NUM_OPS>;

static const operations_map OPERATIONS {{
	&machine::set_func,
	&machine::add_func,
	&machine::sub_func,
	&machine::mul_func,
	&machine::mli_func,
	&machine::div_func,
	&machine::dvi_func,
	&machine::mod_func,
	&machine::mdi_func,
	&machine::and_func,
	&machine::bor_func,
	&machine::xor_func,
    &machine::shr_func,
	&machine::asr_func,
	&machine::shl_func,

	&machine::ifb_func,
	&machine::ifc_func,
	&machine::ife_func,
	&machine::ifn_func,
	&machine::ifg_func,
    &machine::ifa_func,
    &machine::ifl_func,
    &machine::ifu_func,

    &machine::adx_func,
    &machine::sbx_func,

    &machine::sti_func,
    &machine::std_func,
}};

}

#endif /* REGISTER_MACHINE_HPP */
