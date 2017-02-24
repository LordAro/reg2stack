#ifndef STACK_MACHINE_HPP
#define STACK_MACHINE_HPP

#include <array>
#include <boost/optional.hpp>
#include <map>
#include <stack>
#include <vector>

namespace j5 {

enum class op_t {
	NOP,
	SET,
	DROP,
	LOAD,
	STORE,
	DUP,
	SWAP,
	INC,
	DEC,
	ADD,
	SUB,
	ADDC,
	SUBC,
	AND,
	OR,
	XOR,
	NOT,
	SHR,
	SHL,

	TEQ,
	TGT,
	BRZERO, //TODO: find other branches/tests
	// TODO: COPYn ?
	// TODO: TUCKn ?
	// TODO: RSDn/RSUn - rotate n stack items

	STOP,
	OUT,
	NUM_OPS,
};

static const std::array<std::string, (size_t)op_t::NUM_OPS> OP_T_STR{{
	"NOP",
	"SET",
	"DROP",
	"LOAD",
	"STORE",
	"DUP",
	"SWAP",
	"INC",
	"DEC",
	"ADD",
	"SUB",
	"ADDC",
	"SUBC",
	"AND",
	"OR",
	"XOR",
	"NOT",
	"SHR",
	"SHL",
	"TEQ",
	"TGT",
	"BrZero",
	"STOP",
	"OUT",
}};

using operand_t = boost::optional<uint16_t>;

struct instruction {
	std::string label;
	op_t code;
	operand_t op;
};

std::ostream& operator<<(std::ostream& os, const instruction& ins);

inline instruction make_instruction(op_t code, operand_t op = boost::none, std::string label = "")
{
	return {label, code, op};
}

using program = std::vector<instruction>;

program tokenise_source(const std::string &source);

class machine {
public:
	void run(const program &prog, bool verbose);
	std::string register_dump();

private:
	uint16_t pc;
	bool terminate;
	std::stack<uint16_t> stack;
	std::array<uint16_t, 0x10000> mem;
	program cur_prog;

	using opfunc_t = std::function<void(machine*)>;
	using op_map = std::map<op_t, opfunc_t>;

	static const op_map OPERATIONS;

	void binop_func(std::function<uint16_t(uint16_t, uint16_t)> op);

	void set_func(uint16_t v);
	void load_func();
	void store_func();
	void swap_func();
	void out_func();
};

}
#endif /* STACK_MACHINE_HPP */
