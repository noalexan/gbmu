#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Joypad {
private:
	GameBoy &gameboy;

	u8 input = 0;

public:
	Joypad(GameBoy &);
	virtual ~Joypad();

	u8 &getInput() { return input; }
};
