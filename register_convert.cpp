#include "register_convert.hpp"

/**
 * Converts register to a memory address for the stack machine to use.
 * Stack "usable" memory starts at 0x2000, so reverse copy the registers into
 * 0x1FFF decrementing as memory storage.
 * @param r Register to convert.
 * @return Stack memory address to use.
 */
uint16_t reg2memaddr(dcpu16::reg_t r)
{
	return 0x2000 - (static_cast<size_t>(r) + 1);
}

/**
 * Takes an "array type" and puts the index of it on the stack
 */
prog_snippet index_on_stack(dcpu16::operand_t x)
{
	assert(x.which() == 0); // arrays are, sadly, strings
	std::string op = boost::get<std::string>(x);
	if (!dcpu16::is_array_type(op)) {
		throw "Attempted to load a label onto the stack" + op;
	}

	prog_snippet ret;
	std::string interior = op.substr(1, op.length() - 2);
	dcpu16::operand_t i = dcpu16::get_operand(interior);
	switch (i.which()) {
		case 0: {
			size_t pos = interior.find('+');
			if (pos == std::string::npos) {
				throw "String memory addressing attempted";
			}
			auto p1 = value_on_stack(dcpu16::get_operand(interior.substr(0, pos)));
			auto p2 = value_on_stack(dcpu16::get_operand(interior.substr(pos + 1)));
			ret.insert(ret.end(), p1.begin(), p1.end());
			ret.insert(ret.end(), p2.begin(), p2.end());
			ret.emplace_back(j5::make_instruction(j5::op_t::ADD));
			break;
		}
		case 1: {
			uint16_t val = reg2memaddr(boost::get<dcpu16::reg_t>(i));
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			ret.emplace_back(j5::make_instruction(j5::op_t::LOAD)); // deref
			break;
		}
		case 2: {
			uint16_t val = boost::get<uint16_t>(i);
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			break;
		}
	}
	return ret;
}

/*
 * Pushes the *address* of a register operand onto the stack (no other sideeffects)
 */
prog_snippet address_on_stack(dcpu16::operand_t x)
{
	prog_snippet ret;
	switch (x.which()) {
		case 0: // string
			ret = index_on_stack(x);
			break;
		case 1: {
			uint16_t val = reg2memaddr(boost::get<dcpu16::reg_t>(x));
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			break;
		}
		case 2:
			uint16_t val = boost::get<uint16_t>(x);
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			break;
	}
	return ret;
}

/*
 * Pushes the value of a register operand onto the stack (no other sideeffects)
 * What happens next is anyone's guess :)
 */
prog_snippet value_on_stack(dcpu16::operand_t x)
{
	prog_snippet ret = address_on_stack(x);
	if (x.which() == 0 || x.which() == 1) {
		ret.emplace_back(j5::make_instruction(j5::op_t::LOAD));
	}
	return ret;
}

prog_snippet set_snippet(const dcpu16::instruction &ins)
{
	// SET PC, x is special
	// TODO: handle numeric (& +/-??)
	if (ins.b.which() == 1 && boost::get<dcpu16::reg_t>(ins.b) == dcpu16::reg_t::PC) {
		if (ins.a.which() == 0) {
			return {j5::make_instruction(j5::op_t::BRANCH, boost::get<std::string>(ins.a))};
		} else if (ins.a.which() == 1 && boost::get<dcpu16::reg_t>(ins.a) == dcpu16::reg_t::PC) { // SET PC, PC
			return {j5::make_instruction(j5::op_t::STOP)};
		}
	}

	if (ins.b.which() == 2) return {}; // setting to literal == nop
	prog_snippet ret;
	auto a_snip = value_on_stack(ins.a);
	auto b_snip = address_on_stack(ins.b);
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

prog_snippet out_snippet(const dcpu16::instruction &ins)
{
	prog_snippet ret = value_on_stack(ins.b);
	ret.emplace_back(j5::make_instruction(j5::op_t::OUT));
	ret.emplace_back(j5::make_instruction(j5::op_t::DROP));
	return ret;
}

// TODO EX register
prog_snippet add_snippet(const dcpu16::instruction &ins)
{
	if (ins.b.which() == 2) {
		return {}; // nop
	}
	prog_snippet b_snip = value_on_stack(ins.b);
	prog_snippet a_snip = value_on_stack(ins.a);
	prog_snippet ret;
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::ADD));

	prog_snippet addr_snip = address_on_stack(ins.b);
	ret.insert(ret.end(), addr_snip.begin(), addr_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

prog_snippet sub_snippet(const dcpu16::instruction &ins)
{
	if (ins.b.which() == 2) {
		return {}; // nop
	}
	prog_snippet b_snip = value_on_stack(ins.b);
	prog_snippet a_snip = value_on_stack(ins.a);
	prog_snippet ret;
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::SUB));

	prog_snippet addr_snip = address_on_stack(ins.b);
	ret.insert(ret.end(), addr_snip.begin(), addr_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

prog_snippet if_snippet(const dcpu16::instruction &ins, j5::op_t test_op, bool invert)
{
	prog_snippet b_snip = value_on_stack(ins.b);
	prog_snippet a_snip = value_on_stack(ins.a);
	prog_snippet ret;

	/* Swapped as the LT/GT instructions do top <> next
	 * Other instructions don't care */
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());

	ret.emplace_back(j5::make_instruction(test_op));
	ret.emplace_back(j5::make_instruction(j5::op_t::DROP));
	ret.emplace_back(j5::make_instruction(j5::op_t::DROP));
	if (invert) {
		/* Actual length is handled in the main loop, as this
		 * requires the generated length of the next instruction. */
		ret.emplace_back(j5::make_instruction(j5::op_t::BRZERO, 42));
	} else {
		ret.emplace_back(j5::make_instruction(j5::op_t::BRZERO, 2));
		ret.emplace_back(j5::make_instruction(j5::op_t::BRANCH, 42));
	}
	return ret;
}

/**
 * Whole point of this program. :)
 * Takes a register instruction and converts to a stack instruction.
 * @param r Register instruction.
 * @return List of stack instructions equivalent to the register instruction.
 */
prog_snippet convert_instruction(const dcpu16::instruction &r)
{
	using namespace std::placeholders;

	static const std::map<dcpu16::op_t, std::function<prog_snippet(const dcpu16::instruction &)>> conv_map {
		{dcpu16::op_t::SET, &set_snippet},
		{dcpu16::op_t::ADD, &add_snippet},
		{dcpu16::op_t::SUB, &sub_snippet},
		{dcpu16::op_t::OUT, &out_snippet},
		{dcpu16::op_t::IFN, std::bind(&if_snippet, _1, j5::op_t::TEQ, true)},
		{dcpu16::op_t::IFG, std::bind(&if_snippet, _1, j5::op_t::TGT, false)},
		{dcpu16::op_t::IFE, std::bind(&if_snippet, _1, j5::op_t::TEQ, false)},
		{dcpu16::op_t::IFL, std::bind(&if_snippet, _1, j5::op_t::TLT, false)},
	};
	auto keyval = conv_map.find(r.code);
	if (keyval != conv_map.end()) {
		auto converted = keyval->second(r);
		if (r.label != "") {
			// TODO: preserve label if prevous conversion resulted in a nop
			converted.front().label = r.label;
		}
		return converted;
	} else {
		throw "Unimplemented conversion of " + dcpu16::OP_T_STR.at((size_t)r.code);
	}
}

j5::program reg2stack(dcpu16::program p)
{
	std::vector<j5::program> snippets;
	for (const auto &i : p) {
		auto snippet = convert_instruction(i);

		/* Deal with generated BRZERO from if statements
		 * requiring the length of the next instruction */
		// TODO: Look at p instead of snippets?
		if (snippets.size() != 0) {
			auto &last = snippets.back();
			auto brzero_idx = std::find_if(last.begin(), last.end(),
					[](const j5::instruction &i){return i.code == j5::op_t::BRZERO;});
			if (brzero_idx != last.end()) {
				brzero_idx->op = snippet.size() + 1 + 2;
			}
		}
		snippets.push_back(snippet);
	}
	j5::program ret;
	for(const auto &v : snippets) ret.insert(ret.end(), v.begin(), v.end());
	return ret;
}
