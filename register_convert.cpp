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
 * Pushes the value of a register operand onto the stack (no other sideeffects)
 * What happens next is anyone's guess :)
 */
prog_snippet operand_on_stack(dcpu16::operand_t x)
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
			ret.emplace_back(j5::make_instruction(j5::op_t::LOAD));
			break;
		}
		case 1: {
			uint16_t val = reg2memaddr(boost::get<dcpu16::reg_t>(x));
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			ret.emplace_back(j5::make_instruction(j5::op_t::LOAD));
			break;
		}
		case 2:
			uint16_t val = boost::get<uint16_t>(x);
			ret.emplace_back(j5::make_instruction(j5::op_t::SET, val));
			break;
	}
	return ret;
}

prog_snippet set_snippet(dcpu16::operand_t b, dcpu16::operand_t a)
{
	prog_snippet ret = operand_on_stack(a);
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
	prog_snippet ret = operand_on_stack(b);
	ret.emplace_back(j5::make_instruction(j5::op_t::OUT));
	ret.emplace_back(j5::make_instruction(j5::op_t::DROP));
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
		{dcpu16::op_t::OUT, &out_snippet},
	};
	auto keyval = conv_map.find(r.code);
	if (keyval != conv_map.end()) {
		return keyval->second(r.b, r.a);
	} else {
		throw "Unimplemented instruction" + dcpu16::OP_T_STR.at((size_t)r.code);
	}
	switch (r.code) {
		case dcpu16::op_t::SET:
		case dcpu16::op_t::ADD:
		case dcpu16::op_t::SUB:
		case dcpu16::op_t::MUL:
		case dcpu16::op_t::MLI:
		case dcpu16::op_t::DIV:
		case dcpu16::op_t::DVI:
		case dcpu16::op_t::MOD:
		case dcpu16::op_t::MDI:
		case dcpu16::op_t::AND:
		case dcpu16::op_t::BOR:
		case dcpu16::op_t::XOR:
		case dcpu16::op_t::SHR:
		case dcpu16::op_t::ASR:
		case dcpu16::op_t::SHL:

		case dcpu16::op_t::IFB:
		case dcpu16::op_t::IFC:
		case dcpu16::op_t::IFE:
		case dcpu16::op_t::IFN:
		case dcpu16::op_t::IFG:
		case dcpu16::op_t::IFA:
		case dcpu16::op_t::IFL:
		case dcpu16::op_t::IFU:
		case dcpu16::op_t::ADX:
		case dcpu16::op_t::SBX:
		case dcpu16::op_t::DAT:
		case dcpu16::op_t::OUT:
		default:
			break;
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
