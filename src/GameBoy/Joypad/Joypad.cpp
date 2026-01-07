#include "Joypad.hpp"
#include "../GameBoy.hpp"

Joypad::Joypad(GameBoy &gb) : gameboy(gb)
{
	gameboy.getMMU().register_handler(
	    0xff00, [this]() { return this->read(0xff00); },
	    [this](u8 value) { this->write(0xff00, value); });
}

Joypad::~Joypad() {}

u8 Joypad::read(u16 address)
{
	if (address == 0xff00) {
		return input;
	}
	return 0xff;
}

void Joypad::write(u16 address, u8 value)
{
	if (address == 0xff00) {
		input = value;
	}
}
