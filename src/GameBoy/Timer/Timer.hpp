#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Timer {
private:
	GameBoy &gameboy;

	u8 &div;
	u8 &tima;
	u8 &tma;
	u8 &tac;

public:
	Timer(GameBoy &);
	virtual ~Timer();

	void tick();

	u8 registers[0x04];
};
