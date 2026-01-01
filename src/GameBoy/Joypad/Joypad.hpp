#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Joypad {
private:
	GameBoy &gameboy;

	u8 p1 = 0;

public:
	Joypad(GameBoy &);
	virtual ~Joypad();

	u8 &getP1() { return p1; }
};
