#include "CPU.hpp"
#include "../GameBoy.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>

CPU::CPU(GameBoy &gb) : gameboy(gb), registers{} {}

CPU::~CPU() {}

u8 &CPU::access(u16 address)
{
	ticks += 4;
	return gameboy.getMMU().access(address);
}

void CPU::step()
{
	if (ticks > 0) {
		ticks--;
		return;
	}

	u8 opcode = imm8();

	switch (opcode) {
	// nop
	case 0x00:
		break;

	// ld r16, imm16
	case 0x01:
	case 0x11:
	case 0x21:
	case 0x31:
		r16(opcode >> 4) = imm16();
		break;

	// ld [r16mem], a
	case 0x02:
	case 0x12:
	case 0x22:
	case 0x32:
		r16mem(opcode >> 4) = registers.a;
		break;

	// ld a, [r16mem]
	case 0x0A:
	case 0x1A:
	case 0x2A:
	case 0x3A:
		registers.a = r16mem(opcode >> 4);
		break;

	// inc r16
	case 0x03:
	case 0x13:
	case 0x23:
	case 0x33: {
		u16 &reg = r16(opcode >> 4);
		setr16(reg, reg + 1);
		break;
	}

	// dec r16

	// add hl, r16
	case 0x09:
	case 0x19:
	case 0x29:
	case 0x39: {
		u16 value   = r16(opcode >> 4);
		u32 result  = registers.hl + value;
		registers.f = (registers.f & ZERO) |
		              ((registers.hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF ? HALF_CARRY : 0) |
		              (result > 0xFFFF ? CARRY : 0);
		setr16(registers.hl, static_cast<u16>(result));
		break;
	}

	// inc r8
	case 0x04:
	case 0x14:
	case 0x24:
	case 0x34:
	case 0x0C:
	case 0x1C:
	case 0x2C:
	case 0x3C: {
		u8 &reg         = r8(opcode >> 3);
		u8  old         = reg;
		r8(opcode >> 3) = reg + 1; // reuse of `r8` to handle hl++
		registers.f =
		    (registers.f & CARRY) | (reg == 0 ? ZERO : 0) | ((old & 0xF) == 0xF ? HALF_CARRY : 0);
		break;
	}

	// dec r8
	case 0x05:
	case 0x15:
	case 0x25:
	case 0x35:
	case 0x0D:
	case 0x1D:
	case 0x2D:
	case 0x3D: {
		u8 &reg         = r8(opcode >> 3);
		u8  old         = reg;
		r8(opcode >> 3) = reg - 1; // reuse of `r8` to handle hl--
		registers.f     = (registers.f & CARRY) | NEGATIVE | (reg == 0 ? ZERO : 0) |
		              ((old & 0xF) == 0 ? HALF_CARRY : 0);
		break;
	}

	// ld r8, imm8
	case 0x06:
	case 0x16:
	case 0x26:
	case 0x36:
	case 0x0E:
	case 0x1E:
	case 0x2E:
	case 0x3E:
		r8(opcode >> 3) = imm8();
		break;

	// rla
	case 0x17: {
		bool carry  = (registers.a & 0x80) != 0;
		registers.a = (registers.a << 1) | (registers.f & CARRY ? 1 : 0);
		registers.f = carry ? CARRY : 0;
		break;
	}

	// rra
	case 0x1F: {
		bool carry  = (registers.a & 0x01) != 0;
		registers.a = (registers.a >> 1) | (registers.f & CARRY ? 0x80 : 0);
		registers.f = carry ? CARRY : 0;
		break;
	}

	// jr imm8
	case 0x18: {
		s8 offset = static_cast<s8>(imm8());
		setr16(registers.pc, registers.pc + offset);
		break;
	}

	// jr cond, imm8
	case 0x20:
	case 0x30:
	case 0x28:
	case 0x38: {
		s8 offset = static_cast<s8>(imm8());
		if (cond(opcode >> 3))
			setr16(registers.pc, registers.pc + offset);
		break;
	}

	// ld r8, r8
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
	case 0x4F:
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6A:
	case 0x6B:
	case 0x6C:
	case 0x6D:
	case 0x6E:
	case 0x6F:
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
		r8(opcode >> 3) = r8(opcode);
		break;

	// add a, r8
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87: {
		u8 value    = r8(opcode);
		registers.f = ((registers.a & 0xF) + (value & 0xF) > 0xF ? HALF_CARRY : 0) |
		              (registers.a + value > 0xFF ? CARRY : 0);
		registers.a += value;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// adc a, r8
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	case 0x8C:
	case 0x8D:
	case 0x8E:
	case 0x8F: {
		u8 value    = r8(opcode);
		u8 carry    = (registers.f & CARRY) ? 1 : 0;
		registers.f = ((registers.a & 0xF) + (value & 0xF) + carry > 0xF ? HALF_CARRY : 0) |
		              (registers.a + value + carry > 0xFF ? CARRY : 0);
		registers.a += value + carry;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// sub a, r8
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97: {
		u8 value    = r8(opcode);
		registers.f = NEGATIVE | ((registers.a & 0xF) < (value & 0xF) ? HALF_CARRY : 0) |
		              (registers.a < value ? CARRY : 0);
		registers.a -= value;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// sbc a, r8
	case 0x98:
	case 0x99:
	case 0x9A:
	case 0x9B:
	case 0x9C:
	case 0x9D:
	case 0x9E:
	case 0x9F: {
		u8 value    = r8(opcode);
		u8 carry    = (registers.f & CARRY) ? 1 : 0;
		registers.f = NEGATIVE | ((registers.a & 0xF) < (value & 0xF) + carry ? HALF_CARRY : 0) |
		              (registers.a < value + carry ? CARRY : 0);
		registers.a -= value + carry;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// and a, r8
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	case 0xA4:
	case 0xA5:
	case 0xA6:
	case 0xA7:
		registers.a &= r8(opcode);
		registers.f = HALF_CARRY | (registers.a == 0 ? ZERO : 0);
		break;

	// xor a, r8
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
		registers.a ^= r8(opcode);
		registers.f = (registers.a == 0 ? ZERO : 0);
		break;

	// cp a, r8
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf: {
		u8 value    = r8(opcode);
		registers.f = NEGATIVE | (registers.a - value == 0 ? ZERO : 0) |
		              ((registers.a & 0xF) < (value & 0xF) ? HALF_CARRY : 0) |
		              (registers.a < value ? CARRY : 0);
		break;
	}

	// or a, r8
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
		registers.a |= r8(opcode);
		registers.f = (registers.a == 0 ? ZERO : 0);
		break;

	// add a, imm8
	case 0xc6: {
		u8 value    = imm8();
		registers.f = ((registers.a & 0xF) + (value & 0xF) > 0xF ? HALF_CARRY : 0) |
		              (registers.a + value > 0xFF ? CARRY : 0);
		registers.a += value;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// adc a, imm8
	case 0xce: {
		u8 value    = imm8();
		u8 carry    = (registers.f & CARRY) ? 1 : 0;
		registers.f = ((registers.a & 0xF) + (value & 0xF) + carry > 0xF ? HALF_CARRY : 0) |
		              (registers.a + value + carry > 0xFF ? CARRY : 0);
		registers.a += value + carry;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// sub a, imm8
	case 0xd6: {
		u8 value    = imm8();
		registers.f = NEGATIVE | ((registers.a & 0xF) < (value & 0xF) ? HALF_CARRY : 0) |
		              (registers.a < value ? CARRY : 0);
		registers.a -= value;
		registers.f |= (registers.a == 0 ? ZERO : 0);
		break;
	}

	// sbc a, imm8

	// and a, imm8
	case 0xe6:
		registers.a &= imm8();
		registers.f = HALF_CARRY | (registers.a == 0 ? ZERO : 0);
		break;

	// xor a, imm8
	case 0xee:
		registers.a ^= imm8();
		registers.f = (registers.a == 0 ? ZERO : 0);
		break;

	// or a, imm8
	case 0xf6:
		registers.a |= imm8();
		registers.f = (registers.a == 0 ? ZERO : 0);
		break;

	// cp a, imm8
	case 0xfe: {
		u8 value    = imm8();
		registers.f = NEGATIVE | (registers.a - value == 0 ? ZERO : 0) |
		              ((registers.a & 0xF) < (value & 0xF) ? HALF_CARRY : 0) |
		              (registers.a < value ? CARRY : 0);
		break;
	}

	// ret cond
	case 0xC0:
	case 0xD0:
	case 0xC8:
	case 0xD8: {
		ticks += 4; // I don't know why but it takes an extra 4 cycles
		            // https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#ret-cc
		if (cond(opcode >> 3)) {
			u8 low  = pop();
			u8 high = pop();
			setr16(registers.pc, low | (high << 8));
		}
		break;
	}

	// ret
	case 0xC9: {
		u8 low  = pop();
		u8 high = pop();
		setr16(registers.pc, low | (high << 8));
		break;
	}

	// reti

	// jp cond, imm16
	case 0xc2:
	case 0xd2:
	case 0xca:
	case 0xda: {
		u16 address = imm16();
		if (cond(opcode >> 3)) {
			setr16(registers.pc, address);
		}
		break;
	}

	// jp imm16
	case 0xc3: {
		u16 address = imm16();
		setr16(registers.pc, address);
		break;
	}

	// jp hl
	case 0xe9:
		registers.pc = registers.hl;
		break;

	// call cond, imm16
	case 0xc4:
	case 0xd4:
	case 0xcc:
	case 0xdc: {
		u16 address = imm16();
		if (cond(opcode >> 3)) {
			push(registers.pc >> 8);
			push(registers.pc);
			setr16(registers.pc, address);
		}
		break;
	}

	// call imm16
	case 0xcd: {
		u16 address = imm16();
		push(registers.pc >> 8);
		push(registers.pc);
		setr16(registers.pc, address);
		break;
	}

	// rst tgt3

	// pop r16stk
	case 0xC1:
	case 0xD1:
	case 0xE1:
	case 0xF1: {
		u8   low  = pop();
		u8   high = pop();
		u16 &reg  = r16stk(opcode >> 4);
		reg       = low | (high << 8);
		break;
	}

	// push r16stk
	case 0xC5:
	case 0xD5:
	case 0xE5:
	case 0xF5: {
		u16 value = r16stk(opcode >> 4);
		push(value >> 8);
		push(value);
		break;
	}

	case 0xcb: {
		opcode = imm8();

		switch (opcode) {
		// rl r8
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17: {
			u8   reg    = r8(opcode);
			bool carry  = (reg & 0x80) != 0;
			u8   result = (reg << 1) | (registers.f & CARRY ? 1 : 0);
			r8(opcode)  = result;
			registers.f = (carry ? CARRY : 0) | (result == 0 ? ZERO : 0);
			break;
		}

		// rr r8
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F: {
			u8   reg    = r8(opcode);
			bool carry  = reg & 1;
			u8   result = (reg >> 1) | (registers.f & CARRY ? 0x80 : 0);
			r8(opcode)  = result;
			registers.f = (carry ? CARRY : 0) | (result == 0 ? ZERO : 0);
			break;
		}

		// sla r8

		// sra r8

		// swap r8
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37: {
			u8 reg      = r8(opcode);
			r8(opcode)  = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);
			registers.f = (reg == 0 ? ZERO : 0);
			break;
		}

		// srl r8
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F: {
			u8   reg    = r8(opcode);
			bool carry  = (reg & 0x01) != 0;
			u8   result = reg >> 1;
			r8(opcode)  = result;
			registers.f = (carry ? CARRY : 0) | (result == 0 ? ZERO : 0);
			break;
		}

		// bit b3, r8
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
			registers.f =
			    (registers.f & CARRY) | HALF_CARRY | (r8(opcode) & b3(opcode >> 3) ? 0 : ZERO);
			break;

		// res b3, r8
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7: {
			u8 value   = r8(opcode);
			r8(opcode) = value & ~b3(opcode >> 3);
			break;
		}

		// set b3, r8
		case 0xC0:
		case 0xC1:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
		case 0xE0:
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF: {
			u8 value   = r8(opcode);
			r8(opcode) = value | b3(opcode >> 3);
			break;
		}

		default: {
			std::stringstream ss;
			ss << "Unknown CB instruction: 0x" << std::hex << (int)opcode;
			throw std::runtime_error(ss.str());
		}
		}
		break;
	}

	// ldh [c], a
	case 0xE2:
		access(0xFF00 + registers.c) = registers.a;
		break;

	// ldh [imm8], a
	case 0xE0:
		access(0xFF00 + imm8()) = registers.a;
		break;

	// ld [imm16], a
	case 0xEA:
		access(imm16()) = registers.a;
		break;

	// ldh a, [c]
	case 0xF2:
		registers.a = access(0xFF00 + registers.c);
		break;

	// ldh a, [imm8]
	case 0xF0:
		registers.a = access(0xFF00 + imm8());
		break;

	// ld a, [imm16]
	case 0xFA:
		registers.a = access(imm16());
		break;

	// add sp, imm8

	// ld hl, sp + imm8

	// ld sp, hl

	// di
	case 0xF3:
		interrupt_enabled = false;
		break;

	// ei
	case 0xFB:
		interrupt_enabled = true;
		break;

	default: {
		std::stringstream ss;
		ss << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0')
		   << (int)opcode;
		throw std::runtime_error(ss.str());
	}
	}
}
