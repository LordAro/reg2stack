#include <iostream>

#include "optimise.hpp"

j5::program optimise_addone(j5::program prog)
{
	for (size_t i = 0; i < prog.size() - 1; i++) {
		auto &a = prog[i];
		auto &b = prog[i + 1];
		if (a.code == j5::op_t::SET && a.op.which() == 1 && boost::get<uint16_t>(a.op) == 1 && b.code == j5::op_t::ADD) {
			a = j5::make_instruction(j5::op_t::INC);
			prog.erase(prog.begin() + i + 1);
			i += 1;
		}
	}
	return prog;
}

j5::program optimise_subone(j5::program prog)
{
	for (size_t i = 0; i < prog.size() - 1; i++) {
		auto &a = prog[i];
		auto &b = prog[i + 1];
		if (a.code == j5::op_t::SET && a.op.which() == 1 && boost::get<uint16_t>(a.op) == 1 && b.code == j5::op_t::SUB) {
			a = j5::make_instruction(j5::op_t::DEC);
			prog.erase(prog.begin() + i + 1);
			i += 1;
		}
	}
	return prog;
}

j5::program optimise_testzero(j5::program prog)
{
	for (size_t i = 0; i < prog.size() - 1; i++) {
		auto &a = prog[i];
		auto &b = prog[i + 1];
		if (a.code == j5::op_t::SET && a.op.which() == 0 && boost::get<uint16_t>(a.op) == 1 && b.code == j5::op_t::TEQ) {
			a = j5::make_instruction(j5::op_t::TSZ);
			prog.erase(prog.begin() + i + 1);
			i += 1;
		}
	}
	return prog;
}

j5::program optimise_storeload(j5::program prog)
{
	for (size_t i = 0; i < prog.size() - 3; i++) {
		auto &a = prog[i];
		auto &b = prog[i + 1];
		auto &c = prog[i + 2];
		auto &d = prog[i + 3];
		if (a.code == j5::op_t::SET && c.code == j5::op_t::SET && b.code == j5::op_t::STORE && d.code == j5::op_t::LOAD && a.op == c.op) {
			a = j5::make_instruction(j5::op_t::DUP);
			b = j5::make_instruction(j5::op_t::SET, c.op);
			c = j5::make_instruction(j5::op_t::STORE);
			prog.erase(prog.begin() + i + 3);
			i += 3;
		}
	}
	return prog;
}

j5::program peephole_optimise(j5::program prog)
{
	prog = optimise_addone(prog);
	prog = optimise_subone(prog);
	prog = optimise_testzero(prog);
	prog = optimise_storeload(prog);

	//std::cout << "Optimised snippet:\n";
	//for (const auto &i : prog) std::cout << i << '\n';
	//std::string s;
	//std::cin >> s;
	return prog;
}

j5::program stack_schedule(j5::program prog)
{
	return prog;
}
