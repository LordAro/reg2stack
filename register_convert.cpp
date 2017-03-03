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

/*
 * Pushes the *address* of a register operand onto the stack (no other sideeffects)
 */
prog_snippet operand_addr_on_stack(dcpu16::operand_t x)
{
	prog_snippet ret;
	switch (x.which()) {
		case 0: {// string
			// continuing much sad.
			std::string op = boost::get<std::string>(x);
			if (op.size() <= 2 || op.front() != '[' || op.back() != ']') { // array val
				throw "Unrecognised string value";
			}
			std::string interior = op.substr(1, op.length() - 2);
			dcpu16::operand_t i = dcpu16::get_operand(interior);
			// TODO: Unnest this section?
			switch (i.which()) {
				case 0: throw "String memory addressing attempted"; // plsno
					//size_t pos = op.find('+'); // TODO
					//std::string p1 = op.substr(0, pos);
					//std::string p2 = op.substr(pos + 1);
				case 1: {
					uint16_t val = reg2memaddr(boost::get<dcpu16::reg_t>(i));
					ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
					ret.emplace_back(j5::make_instruction(j5::op_t::LOAD)); // additional deref
					break;
				}
				case 2: {
					uint16_t val = boost::get<uint16_t>(i);
					ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
					break;
				}
			}
			break;
		}
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
prog_snippet operand_val_on_stack(dcpu16::operand_t x)
{
	prog_snippet ret = operand_addr_on_stack(x);
	if (x.which() == 0 || x.which() == 1) {
		ret.emplace_back(j5::make_instruction(j5::op_t::LOAD));
	}
	return ret;
}

prog_snippet set_snippet(dcpu16::operand_t b, dcpu16::operand_t a)
{
	prog_snippet ret = operand_val_on_stack(a);
	switch (b.which()) {
		case 0: // string
			throw "Setting to string NYI";
		case 1: { // reg
			uint16_t addr = reg2memaddr(boost::get<dcpu16::reg_t>(b));
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, addr));
			break;
		}
		case 2: // literal
			return {}; // nop
	}
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

prog_snippet out_snippet(dcpu16::operand_t b, dcpu16::operand_t)
{
	prog_snippet ret = operand_val_on_stack(b);
	ret.emplace_back(j5::make_instruction(j5::op_t::OUT));
	ret.emplace_back(j5::make_instruction(j5::op_t::DROP));
	return ret;
}

// TODO EX register
prog_snippet add_snippet(dcpu16::operand_t b, dcpu16::operand_t a)
{
	if (b.which() == 2) {
		return {}; // nop
	}
	prog_snippet b_snip = operand_val_on_stack(b);
	prog_snippet a_snip = operand_val_on_stack(a);
	prog_snippet ret;
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::ADD));

	prog_snippet addr_snip = operand_addr_on_stack(b);
	ret.insert(ret.end(), addr_snip.begin(), addr_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

prog_snippet sub_snippet(dcpu16::operand_t b, dcpu16::operand_t a)
{
	if (b.which() == 2) {
		return {}; // nop
	}
	prog_snippet b_snip = operand_val_on_stack(b);
	prog_snippet a_snip = operand_val_on_stack(a);
	prog_snippet ret;
	ret.insert(ret.end(), b_snip.begin(), b_snip.end());
	ret.insert(ret.end(), a_snip.begin(), a_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::SUB));

	prog_snippet addr_snip = operand_addr_on_stack(b);
	ret.insert(ret.end(), addr_snip.begin(), addr_snip.end());
	ret.emplace_back(j5::make_instruction(j5::op_t::STORE));
	return ret;
}

/**
 * Whole point of this program. :)
 * Takes a register instruction and converts to a stack instruction.
 * @param r Register instruction.
 * @return List of stack instructions equivalent to the register instruction.
 */
prog_snippet instruction_convert(const dcpu16::instruction &r)
{
	static const std::map<dcpu16::op_t, std::function<prog_snippet(dcpu16::operand_t, dcpu16::operand_t)>> conv_map {
		{dcpu16::op_t::SET, &set_snippet},
		{dcpu16::op_t::ADD, &add_snippet},
		{dcpu16::op_t::SUB, &sub_snippet},
		{dcpu16::op_t::OUT, &out_snippet},
	};
	auto keyval = conv_map.find(r.code);
	if (keyval != conv_map.end()) {
		return keyval->second(r.b, r.a);
	} else {
		throw "Unimplemented instruction " + dcpu16::OP_T_STR.at((size_t)r.code);
	}
}

j5::program reg2stack(dcpu16::program p)
{
	j5::program ret;
	for (const auto &i : p) {
		auto snippet = instruction_convert(i);
		ret.insert(ret.end(), snippet.begin(), snippet.end());
	}
	return ret;
}
