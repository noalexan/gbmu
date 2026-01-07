#include "Timer.hpp"
#include "../GameBoy.hpp"

Timer::Timer(GameBoy &gb) : gameboy(gb), tima(registers[1]), tma(registers[2]), tac(registers[3])
{
	for (u16 addr = 0xff04; addr <= 0xff07; addr++) {
		gameboy.getMMU().register_handler(
		    addr, [this, addr]() { return this->read(addr); },
		    [this, addr](u8 value) { this->write(addr, value); });
	}
}

Timer::~Timer() {}

u8 Timer::read(u16 address)
{
	switch (address) {
	case 0xff04:
		return (div_counter >> 8) & 0xff;
	default:
		return registers[address - 0xff04];
	}
}

void Timer::write(u16 address, u8 value)
{
	switch (address) {
	case 0xff04:
		div_counter = 0;
		break;
	default:
		registers[address - 0xff04] = value;
		break;
	}
}

void Timer::tick()
{
	div_counter++;

	if (tac & 0x04) {
		int threshold;
		switch (tac & 0x03) {
		case 0:
			threshold = 1024;
			break;
		case 1:
			threshold = 16;
			break;
		case 2:
			threshold = 64;
			break;
		case 3:
			threshold = 256;
			break;
		}

		timer_counter++;
		if (timer_counter >= threshold) {
			timer_counter = 0;
			if (++tima == 0) {
				gameboy.getCPU().requestInterrupt(CPU::Interrupt::TIMER);
				tima = tma;
			}
		}
	}
}
