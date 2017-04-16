#include <iostream>
#include <numeric>

#include "optimise.hpp"
#include "util.hpp"

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
	for (size_t i = 0; prog.size() > 3 && i < prog.size() - 3; i++) {
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

	return prog;
}

j5::program stack_schedule(j5::program prog)
{
	std::vector<std::pair<size_t, size_t>> pairs;
	for (size_t i = 0; i < prog.size() - 1; i++) {
		if (prog[i].code != j5::op_t::SET || prog[i + 1].code != j5::op_t::STORE) continue;
		for (size_t j = i + 2; j < prog.size() - 1; j++) {
			if (prog[j].code != j5::op_t::SET || prog[j + 1].code != j5::op_t::LOAD || prog[i].op != prog[j].op) continue;

			log<LOG_DEBUG2>("Found pair: ", prog[i], " - distance: ", j-i, "(", i, "->", j, ")");
			pairs.push_back({i, j});
			break;
		}
	}
	// sort by shortest distance
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b){return a.second-a.first < b.second-b.first;});

	for (auto p_it = pairs.begin(); p_it != pairs.end(); ++p_it) {
		size_t i = p_it->first;
		size_t j = p_it->second;
		assert(j > i);

		int stack_depth_start = std::count_if(pairs.begin(), pairs.end(), [i](auto &p){
				return p.first < i && i < p.second;
		});
		int stack_depth_end = std::count_if(pairs.begin(), pairs.end(), [j](auto &p){
				return p.first < j && j < p.second;
		});
		if (stack_depth_start > 2 || stack_depth_end > 2) continue;

		j5::op_t ins;
		switch (stack_depth_start) {
			case 0: ins = j5::op_t::DUP; break;
			case 1: ins = j5::op_t::TUCK2; break;
			case 2: ins = j5::op_t::TUCK3; break;
			default: throw "Not reached";
		}

		// reversed because of distances
		prog.erase(prog.begin() + j + 1);
		prog.erase(prog.begin() + j);
		prog.insert(prog.begin() + i, j5::make_instruction(ins));

		int stack_diff = std::accumulate(
				prog.begin() + i + 3, // "dup", set, store
				prog.begin() + j + 1, // +1
				0,
				[](const auto &a, const auto &b) {
					return a + j5::machine::STACK_DIFF.at(b.code);
				}
		);
		size_t unfinished_pairs = std::count_if(pairs.begin(), pairs.end(),
				[i, j](auto &p) { return i < p.first && p.first < j && j < p.second; });

		assert(stack_diff + unfinished_pairs >= 0);
		if (stack_diff > 2) throw "Not reached"; // TODO: not attempt?
		switch (stack_diff) {
			case 1:
				prog.insert(prog.begin() + j + 1, j5::make_instruction(j5::op_t::SWAP));
				break;
			case 2:
				prog.insert(prog.begin() + j + 1, j5::make_instruction(j5::op_t::RSD3));
				break;
		}

		/* adjust upcoming pairs */
		for (auto tmp = p_it + 1; tmp != pairs.end(); ++tmp) {
			if (tmp->first >= i) tmp->first += 1;
			if (tmp->second >= i) tmp->second += 1;
			if (tmp->first >= j) tmp->first -= 2;
			if (tmp->second >= j) tmp->second -= 2;
			if (stack_diff > 0 && tmp->first > j) tmp->first += 1;
			if (stack_diff > 0 && tmp->second > j) tmp->second += 1;
		}
	}
	return prog;
}
