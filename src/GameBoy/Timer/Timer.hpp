#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Timer {
private:
	GameBoy &gameboy;

	u16 div_counter = 0;

	u8 div  = 0x00; // DIV (will be managed by div_counter)
	u8 tima = 0x00; // TIMA - Timer counter
	u8 tma  = 0x00; // TMA - Timer modulo
	u8 tac  = 0x00; // TAC - Timer control

	int timer_counter = 0;

public:
	Timer(GameBoy &);
	virtual ~Timer();

	void tick();
	u8   read(u16 address);
	void write(u16 address, u8 value);
};
