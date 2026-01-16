#include "Joypad.hpp"
#include "../GameBoy.hpp"

Joypad::Joypad(GameBoy &_gb) : gb(_gb)
{
	gb.getMMU().register_handler(
	    0xff00, [this](u16) { return read_byte(0xff00); },
	    [this](u16, u8 value) { write_byte(0xff00, value); });
}

Joypad::~Joypad() {}

u8 Joypad::read_byte(u16 address)
{
	u8 result = p1 & 0x30;

	if ((p1 & P1::BUTTONS) == 0) {
		result |= (a ? 0 : 1) | (b ? 0 : 1 << 1) | (select ? 0 : 1 << 2) | (start ? 0 : 1 << 3);
	} else if ((p1 & P1::DPAD) == 0) {
		result |= (right ? 0 : 1) | (left ? 0 : 1 << 1) | (up ? 0 : 1 << 2) | (down ? 0 : 1 << 3);
	} else {
		result |= 0x0F;
	}

	return result | 0xC0;
}

void Joypad::write_byte(u16 address, u8 value) { p1 = (value & 0x30) | (p1 & 0xCF); }

void Joypad::press(enum Input input)
{
	switch (input) {
	case START:
		start = true;
		break;
	case SELECT:
		select = true;
		break;
	case B:
		b = true;
		break;
	case A:
		a = true;
		break;
	case DOWN:
		down = true;
		break;
	case UP:
		up = true;
		break;
	case LEFT:
		left = true;
		break;
	case RIGHT:
		right = true;
		break;
	}

	gb.getCPU().requestInterrupt(CPU::Interrupt::JOYPAD);
}

void Joypad::release(enum Input input)
{
	switch (input) {
	case START:
		start = false;
		break;
	case SELECT:
		select = false;
		break;
	case B:
		b = false;
		break;
	case A:
		a = false;
		break;
	case DOWN:
		down = false;
		break;
	case UP:
		up = false;
		break;
	case LEFT:
		left = false;
		break;
	case RIGHT:
		right = false;
		break;
	}
}
