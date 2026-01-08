#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Joypad {
private:
	GameBoy &gameboy;

	enum P1 { BUTTONS = 1 << 5, DPAD = 1 << 4 };

	u8 p1 = 0x00;

	bool start = false, select = false, b = false, a = false;
	bool down = false, up = false, left = false, right = false;

public:
	Joypad(GameBoy &);
	virtual ~Joypad();

	u8   read(u16 address);
	void write(u16 address, u8 value);

	enum Input { START, SELECT, B, A, DOWN, UP, LEFT, RIGHT };

	void press(enum Input);
	void release(enum Input);
};
