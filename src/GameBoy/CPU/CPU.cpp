#include <iostream>
#include <iomanip>
#include <sstream>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/CPU/CPU.hpp>

CPU::CPU(GameBoy &gb) : _gb(gb), dots(0)
{
	std::cout << "new CPU" << std::endl;
	_pc = 0x0000;
}

CPU::~CPU()
{
	std::cout << "CPU deleted" << std::endl;
}

void CPU::set_flags(u8 flags)
{
	_af.low |= flags;
}

void CPU::unset_flags(u8 flags)
{
	_af.low &= ~flags;
}

u8 &CPU::access(u16 address)
{
	dots += 4;
	return _gb.mmu.access(address);
}

void CPU::set_r16(u16 &r16, u16 value)
{
	dots += 4;
	r16 = value;
}

void CPU::push(u16 value)
{
	_sp--;
	access(_sp--) = value >> 8;
	access(_sp) = value;
	dots += 4; // because Stack Pointer can't pre-decrement
}

u16 CPU::pop()
{
	u16 value;
	value = access(_sp++);
	value |= access(_sp++) << 8;
	return value;
}

u8 &CPU::get_r8(u8 index)
{
	switch (index & 0b111)
	{
	case 0:
		return _bc.high;
	case 1:
		return _bc.low;
	case 2:
		return _de.high;
	case 3:
		return _de.low;
	case 4:
		return _hl.high;
	case 5:
		return _hl.low;
	case 6:
		return access(_hl);
	default:
		return _af.high;
	}
}

u16 &CPU::get_r16(u8 index)
{
	switch (index & 0b11)
	{
	case 0:
		return _bc;
	case 1:
		return _de;
	case 2:
		return _hl;
	default:
		return _sp;
	}
}

u16 &CPU::get_r16stk(u8 index)
{
	switch (index & 0b11)
	{
	case 0:
		return _bc;
	case 1:
		return _de;
	case 2:
		return _hl;
	default:
		return _af;
	}
}

u16 CPU::get_r16mem(u8 index)
{
	switch (index & 0b11)
	{
	case 0:
		return _bc;
	case 1:
		return _de;
	case 2:
		return _hl++;
	default:
		return _hl--;
	}
}

bool CPU::get_cond(u8 index)
{
	switch (index & 0b11)
	{
	case 0:
		return !(_af.low & FLAGS::ZERO);
	case 1:
		return _af.low & FLAGS::ZERO;
	case 2:
		return !(_af.low & FLAGS::CARRY);
	default:
		return _af.low & FLAGS::CARRY;
	}
}

u8 CPU::get_b3(u8 index)
{
	return 1 << (index & 0b111);
}

// u16 CPU::get_tgt3(u8 index)
// {
// }

u8 CPU::get_imm8()
{
	return access(_pc++);
}

u16 CPU::get_imm16()
{
	return get_imm8() | get_imm8() << 8;
}

void CPU::tick()
{
	if (dots > 0)
	{
		dots--;
		return;
	}

	u8 byte = get_imm8();

	switch (byte)
	{
	// Block 0

	// nop
	case 0x00:
		break;

	// ld r16, imm16
	case 0x11:
	case 0x21:
	case 0x31:
		get_r16(byte >> 4) = get_imm16();
		break;

	// ld [r16mem], a
	case 0x22:
	case 0x32:
		access(get_r16mem(byte >> 4)) = _af.high;
		break;

	// ld a, [r16mem]
	case 0x1A:
		_af.high = access(get_r16mem(byte >> 4));
		break;

	// ld [imm16], sp

	// else if (byte == 0b00001000)
	// {
	// 	u16 address = get_imm16();
	// 	access(address) = _sp.low;
	// 	access(address + 1) = _sp.high;
	// }

	// inc r16
	case 0x03:
	case 0x13:
	case 0x23:
	case 0x33:
	{
		u16 &r16 = get_r16(byte >> 4);
		set_r16(r16, r16 + 1);
		break;
	}

	// dec r16

	// else if ((byte & 0b11001111) == 0b00001011)
	// {
	// 	u16 &r16 = get_r16(byte >> 4);
	// 	set_r16(r16, r16 - 1);
	// }

	// add hl, r16

	// // else if ((byte & 0b11001111) == 0b00001001)
	// // {
	// // 	u16 &r16 = get_r16(byte >> 4);
	// // 	set_r16(_hl, _hl + r16);

	// // 	// todo: set flags
	// // }

	// inc r8
	case 0x04:
	case 0x24:
	case 0x0C:
	{
		u8 &r8 = get_r8(byte >> 3);

		if (r8 & 0b1111)
			set_flags(FLAGS::HALF_CARRY);
		else
			unset_flags(FLAGS::HALF_CARRY);

		r8++;

		if (r8 == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		unset_flags(FLAGS::SUBSTRACT);
		break;
	}

	// dec r8
	case 0x05:
	case 0x15:
	case 0x0D:
	case 0x1D:
	case 0x3D:
	{
		u8 &r8 = get_r8(byte >> 3);

		if (~r8 & 0b1111)
			set_flags(FLAGS::HALF_CARRY);
		else
			unset_flags(FLAGS::HALF_CARRY);

		r8--;

		if (r8 == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		set_flags(FLAGS::SUBSTRACT);

		break;
	}

	// ld r8, imm8
	case 0x06:
	case 0x16:
	case 0x0E:
	case 0x1E:
	case 0x2E:
	case 0x3E:
		get_r8(byte >> 3) = get_imm8();
		break;

	// rlca	0	0	0	0	0	1	1	1
	// rrca	0	0	0	0	1	1	1	1

	// rla
	case 0x17:
		if (_af.high & 0b10000000)
			set_flags(FLAGS::CARRY);
		else
			unset_flags(FLAGS::CARRY);

		unset_flags(FLAGS::ZERO | FLAGS::SUBSTRACT | FLAGS::HALF_CARRY);

		_af.high <<= 1;
		break;

	// rra	0	0	0	1	1	1	1	1
	// daa	0	0	1	0	0	1	1	1
	// cpl	0	0	1	0	1	1	1	1
	// scf	0	0	1	1	0	1	1	1
	// ccf	0	0	1	1	1	1	1	1

	// jr imm8
	case 0x18:
	{
		s8 jr = get_imm8();
		set_r16(_pc, _pc + jr);
		break;
	}

	// jr cond, imm8
	case 0x20:
	case 0x28:
	{
		s8 jr = get_imm8();
		if (get_cond(byte >> 3))
			set_r16(_pc, _pc + jr);
		break;
	}

	// stop	0	0	0	1	0	0	0	0

	// Block 1

	// ld r8, r8
	case 0x4F:
	case 0x57:
	case 0x67:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
		get_r8(byte >> 3) = get_r8(byte);
		break;

		// Block 2

	// add a, r8
	case 0x86:
	{
		unset_flags(FLAGS::SUBSTRACT);

		u8 &r8 = get_r8(byte);

		if (0xFF - _af.high < r8)
			set_flags(FLAGS::CARRY);
		else
			unset_flags(FLAGS::CARRY);

		_af.high += r8;

		if (_af.high == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		// todo: set half carry

		break;
	}

	// adc a, r8	1	0	0	0	1	Operand (r8)

	// sub a, r8
	case 0x90:
	{
		u8 &r8 = get_r8(byte);

		s16 sub = _af.high;
		sub -= r8;

		set_flags(FLAGS::SUBSTRACT);

		if (sub == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		if (sub < 0x00)
			set_flags(FLAGS::CARRY);
		else
			unset_flags(FLAGS::CARRY);

		// todo: set half carry

		_af.high -= r8;
		break;
	}

	// sbc a, r8	1	0	0	1	1	Operand (r8)
	// and a, r8	1	0	1	0	0	Operand (r8)

	// xor a, r8
	case 0xAF:
		_af.high ^= get_r8(byte);
		unset_flags(FLAGS::SUBSTRACT | FLAGS::HALF_CARRY | FLAGS::CARRY);
		if (_af.high == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);
		break;

	// or a, r8	1	0	1	1	0	Operand (r8)

	// cp a, r8
	case 0xBE:
	{
		u8 &r8 = get_r8(byte);

		s16 cp = _af.high;
		cp -= r8;

		set_flags(FLAGS::SUBSTRACT);

		if (cp == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		if (cp < 0x00)
			set_flags(FLAGS::CARRY);
		else
			unset_flags(FLAGS::CARRY);

		// todo: set half carry

		break;
	}

	// Block 3

	// add a, imm8	1	1	0	0	0	1	1	0
	// adc a, imm8	1	1	0	0	1	1	1	0
	// sub a, imm8	1	1	0	1	0	1	1	0
	// sbc a, imm8	1	1	0	1	1	1	1	0
	// and a, imm8	1	1	1	0	0	1	1	0
	// xor a, imm8	1	1	1	0	1	1	1	0
	// or a, imm8	1	1	1	1	0	1	1	0

	// cp a, imm8
	case 0xFE:
	{
		s16 cp = _af.high;
		cp -= get_imm8();

		set_flags(FLAGS::SUBSTRACT);

		if (cp == 0x00)
			set_flags(FLAGS::ZERO);
		else
			unset_flags(FLAGS::ZERO);

		if (cp < 0x00)
			set_flags(FLAGS::CARRY);
		else
			unset_flags(FLAGS::CARRY);

		// todo: set half carry

		break;
	}

	// ret cond	1	1	0	Condition (cond)	0	0	0

	// ret
	case 0xC9:
		set_r16(_pc, pop());
		break;

	// reti	1	1	0	1	1	0	0	1
	// jp cond, imm16	1	1	0	Condition (cond)	0	1	0
	// jp imm16	1	1	0	0	0	0	1	1
	// jp hl	1	1	1	0	1	0	0	1
	// call cond, imm16	1	1	0	Condition (cond)	1	0	0

	// call imm16
	case 0xCD:
	{
		u16 address = get_imm16();
		push(_pc);
		set_r16(_pc, address);
		break;
	}

	// rst tgt3	1	1	Target (tgt3)	1	1	1

	// pop r16stk
	case 0xC1:
		get_r16stk(byte >> 4) = pop();
		break;

	// push r16stk
	case 0xC5:
		push(get_r16stk(byte >> 4));
		break;

	// ldh [c], a
	case 0xE2:
		access(0xFF00 + _bc.low) = _af.high;
		break;

	// ldh [imm8], a
	case 0xE0:
		access(0xFF00 + get_imm8()) = _af.high;
		break;

	// ld [imm16], a
	case 0xEA:
		access(get_imm16()) = _af.high;
		break;

		// ldh a, [c]
		_af.high = access(0xFF00 + _bc.low);
		break;

	// ldh a, [imm8]
	case 0xF0:
		_af.high = access(0xFF00 + get_imm8());
		break;

		// ld a, [imm16]
		_af.high = access(get_imm16());
		break;

	// add sp, imm8	1	1	1	0	1	0	0	0
	// ld hl, sp + imm8	1	1	1	1	1	0	0	0
	// ld sp, hl	1	1	1	1	1	0	0	1

	// di	1	1	1	1	0	0	1	1
	// ei	1	1	1	1	1	0	1	1

	// 0xCB prefix
	case 0xCB:
	{
		byte = get_imm8();
		u8 &r8 = get_r8(byte);

		switch (byte)
		{
		// rl r8
		case 0x11:
			if (r8 & 0b10000000)
				set_flags(FLAGS::CARRY);
			else
				unset_flags(FLAGS::CARRY);

			unset_flags(FLAGS::SUBSTRACT | FLAGS::HALF_CARRY);

			r8 <<= 1;

			if (r8 == 0x00)
				set_flags(FLAGS::ZERO);
			else
				unset_flags(FLAGS::ZERO);

		// bit b3, r8
		case 0x7C:
			if (r8 & get_b3(byte >> 3))
				unset_flags(FLAGS::ZERO);
			else
				set_flags(FLAGS::ZERO);

			unset_flags(FLAGS::SUBSTRACT);
			set_flags(FLAGS::HALF_CARRY);

			break;

		default:
			std::stringstream os;

			os << "CPU: Unknown 0xCB-prefixed instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << std::endl;
			throw std::runtime_error(os.str().c_str());
		}
		break;
	}

	default:
		std::stringstream os;

		os << "CPU: Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << std::endl;
		throw std::runtime_error(os.str().c_str());
	}

	// std::cout << "CPU: dots = " << std::dec << dots << std::endl;
}
