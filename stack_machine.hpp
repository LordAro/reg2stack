#ifndef STACK_MACHINE_HPP
#define STACK_MACHINE_HPP

#include <array>
#include <boost/variant.hpp>
#include <map>
#include <stack>
#include <vector>

namespace j5 {

enum class op_t {
	ADD,
	SUB,
	INC,
	DEC,
	AND,
	OR,
	NOT,
	XOR,
	SHR,
	SHL,

	TGT,
	TLT,
	TEQ,
	TSZ,

	SSET,
	SET,
	LOAD,
	STORE,
	BRANCH,
	BRZERO,
	IBRANCH,
	CALL,
	RETURN,
	STOP,
	OUT,

	DROP,
	DUP,
	SWAP,
	RSD3,
	RSU3,
	TUCK2,
	TUCK3,
	COPY3,
	PUSH,
	POP,

	NUM_OPS,
};

static const std::array<std::string, (size_t)op_t::NUM_OPS> OP_T_STR{{
	"ADD",
	"SUB",
	"INC",
	"DEC",
	"AND",
	"OR",
	"NOT",
	"XOR",
	"SHR",
	"SHL",

	"TGT",
	"TLT",
	"TEQ",
	"TSZ",

	"SSET",
	"SET",
	"LOAD",
	"STORE",
	"BRANCH",
	"BRZERO",
	"IBRANCH",
	"CALL",
	"RETURN",
	"STOP",
	"OUT",

	"DROP",
	"DUP",
	"SWAP",
	"RSD3",
	"RSU3",
	"TUCK2",
	"TUCK3",
	"COPY3",
	"PUSH",
	"POP",
}};

using operand_t = boost::variant<boost::blank, uint16_t, std::string>;

struct instruction {
	std::string label;
	op_t code;
	operand_t op;
};

std::ostream& operator<<(std::ostream& os, const instruction& ins);

inline instruction make_instruction(op_t code, operand_t op = boost::blank(), std::string label = "")
{
	return {label, code, op};
}

using program = std::vector<instruction>;

program tokenise_source(const std::string &source);

class machine {
public:
	void run(const program &prog, bool speedlimit);
	std::string register_dump();

	static const std::map<op_t, int> STACK_DIFF;
protected:
	uint16_t pc;
	std::stack<uint16_t> stack;
	std::array<uint16_t, 0x10000> mem;

	// Registers. In a stack machine. Go figure.
	uint8_t flags;
	uint8_t lbr;
	uint8_t gbr;
	uint8_t vba;
	enum class flagbit : uint8_t {
		CARRY = 0,
		ZERO = 1,
		IMODE = 6,
		INTER = 7,
	};

	bool terminate;
	program cur_prog;

	uint16_t run_instruction(const instruction &ins);
	uint16_t run_branch_instruction(const instruction &ins);

	virtual uint16_t find_label(const std::string &l);

	using opfunc_t = std::function<void(machine*)>;
	using op_map = std::map<op_t, opfunc_t>;

	static const op_map OPERATIONS;

	void binop_func(std::function<uint16_t(uint16_t, uint16_t)> op);
	void comp_func(std::function<bool(uint16_t, uint16_t)> op);
	void testzero_func();

	void set_func(uint16_t v);
	void load_func();
	void store_func();
	void swap_func();
	void out_func();
	void tuck2_func();
	void tuck3_func();
	void rsu3_func();
	void rsd3_func();
	void copy3_func();
};

}
#endif /* STACK_MACHINE_HPP */
