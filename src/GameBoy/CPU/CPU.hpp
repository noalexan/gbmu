#pragma once

#include <utils/types.hpp>

class GameBoy;

class CPU
{
public:
	CPU(GameBoy &);
	~CPU();

	void tick();
	int dots;

	u8 interrupt_enable = 0x00;
	u8 interrupt_flag   = 0x00;

private:
	GameBoy &_gb;

	struct Register
	{
		union
		{
			struct
			{
				u8 low;
				u8 high;
			};
			u16 word;
		};

		u16 &operator=(u16 value)
		{
			word = value;
			return word;
		}

		operator u16 &()
		{
			return word;
		}
	};

	enum FLAGS
	{
		ZERO = 1 << 7,
		SUBSTRACT = 1 << 6,
		HALF_CARRY = 1 << 5,
		CARRY = 1 << 4,
	};

	void set_flags(u8 flags);
	void unset_flags(u8 flags);

	bool _ime = false;

	Register _af;
	Register _bc;
	Register _de;
	Register _hl;
	Register _sp;
	Register _pc;

	u8 &access(u16 address);
	void set_r16(u16 &r16, u16 value);

	void push(u16 value);
	u16 pop();

	u8 &get_r8(u8 index);
	u16 &get_r16(u8 index);
	u16 &get_r16stk(u8 index);
	u16 get_r16mem(u8 index);
	bool get_cond(u8 index);
	u8 get_b3(u8 index);
	u16 get_tgt3(u8 index);
	u8 get_imm8();
	u16 get_imm16();
};
