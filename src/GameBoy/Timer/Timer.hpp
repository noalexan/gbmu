#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Timer {
private:
	GameBoy &gameboy;

	u16 div_counter = 0;
	u8 &tima;
	u8 &tma;
	u8 &tac;

	int timer_counter = 0;

public:
	Timer(GameBoy &);
	virtual ~Timer();

	void tick();
	u8   read(u16 address);
	void write(u16 address, u8 value);

	u8 registers[0x04];
};
