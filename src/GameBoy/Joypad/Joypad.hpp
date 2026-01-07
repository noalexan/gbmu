#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Joypad {
private:
	GameBoy &gameboy;

	u8 input = 0xf;

public:
	Joypad(GameBoy &);
	virtual ~Joypad();

	u8   read(u16 address);
	void write(u16 address, u8 value);

	u8 &getInput() { return input; }
};
