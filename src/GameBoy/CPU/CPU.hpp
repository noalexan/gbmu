#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class CPU {
private:
	GameBoy &gameboy;
	int      ticks = 0;

	enum Flag { ZERO = 1 << 7, NEGATIVE = 1 << 6, HALF_CARRY = 1 << 5, CARRY = 1 << 4 };

	struct Registers {
#define REGISTER_PAIR(low, high, word)                                                             \
	union {                                                                                        \
		struct {                                                                                   \
			u8 low;                                                                                \
			u8 high;                                                                               \
		};                                                                                         \
		u16 word;                                                                                  \
	}

		REGISTER_PAIR(f, a, af);
		REGISTER_PAIR(c, b, bc);
		REGISTER_PAIR(e, d, de);
		REGISTER_PAIR(l, h, hl);

#undef REGISTER_PAIR

		u16 sp;
		u16 pc;
	} registers;

	u8 &access(u16 address);

	inline u8  imm8() { return access(registers.pc++); }
	inline u16 imm16() { return imm8() | (imm8() << 8); }

	inline void setr16(u16 &r16, u16 address)
	{
		ticks += 4;
		r16 = address;
	}

	inline u8 &r8(u8 code)
	{
		switch (code & 0b111) {
		case 0b000:
			return registers.b;
		case 0b001:
			return registers.c;
		case 0b010:
			return registers.d;
		case 0b011:
			return registers.e;
		case 0b100:
			return registers.h;
		case 0b101:
			return registers.l;
		case 0b110:
			return access(registers.hl);
		default:
			return registers.a;
		}
	}

	inline u16 &r16(u8 code)
	{
		switch (code & 0b11) {
		case 0b00:
			return registers.bc;
		case 0b01:
			return registers.de;
		case 0b10:
			return registers.hl;
		default:
			return registers.sp;
		}
	}

	inline u8 &r16mem(u8 code)
	{
		switch (code & 0b11) {
		case 0b00:
			return access(registers.bc);
		case 0b01:
			return access(registers.de);
		case 0b10:
			return access(registers.hl++);
		default:
			return access(registers.hl--);
		}
	}

	inline u16 &r16stk(u8 code)
	{
		switch (code & 0b11) {
		case 0b00:
			return registers.bc;
		case 0b01:
			return registers.de;
		case 0b10:
			return registers.hl;
		default:
			return registers.af;
		}
	}

	inline bool cond(u8 code)
	{
		switch (code & 0b11) {
		case 0b00:
			return !(registers.f & ZERO);
		case 0b01:
			return registers.f & ZERO;
		case 0b10:
			return !(registers.f & CARRY);
		default:
			return registers.f & CARRY;
		}
	}

	inline u8 b3(u8 value) { return (1 << (value & 0b111)); }

	inline void push(u8 value) { access(--registers.sp) = value; }

	inline u8 &pop() { return access(registers.sp++); }

public:
	CPU(GameBoy &);
	virtual ~CPU();
	void step();
};
