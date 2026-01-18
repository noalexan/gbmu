#include <GBMU/GameBoy.hpp>
#include <GBMU/Timer.hpp>

using namespace GBMU;

Timer::Timer(GameBoy &_gb) : gb(_gb)
{
	gb.getMMU().register_handler_range(
	    0xff04, 0xff07, [this](u16 addr) { return read_byte(addr); },
	    [this](u16 addr, u8 value) { write_byte(addr, value); });
}

Timer::~Timer() {}

u8 Timer::read_byte(u16 address)
{
	switch (address) {
	case 0xff04:
		return (div_counter >> 8) & 0xff;
	case 0xff05:
		return tima;
	case 0xff06:
		return tma;
	case 0xff07:
		return tac;
	default:
		return 0;
	}
}

void Timer::write_byte(u16 address, u8 value)
{
	switch (address) {
	case 0xff04:
		div_counter = 0;
		break;
	case 0xff05:
		tima = value;
		break;
	case 0xff06:
		tma = value;
		break;
	case 0xff07:
		tac = value;
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

		if (++timer_counter >= threshold) {
			timer_counter = 0;
			if (++tima == 0) {
				gb.getCPU().requestInterrupt(CPU::Interrupt::TIMER);
				tima = tma;
			}
		}
	}
}
