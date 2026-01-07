#include "Serial.hpp"
#include "../GameBoy.hpp"
#include <iostream>

Serial::Serial(GameBoy &gb) : gameboy(gb)
{
	for (u16 addr = 0xff01; addr <= 0xff02; addr++) {
		gameboy.getMMU().register_handler(
		    addr, [this, addr]() { return this->read(addr); },
		    [this, addr](u8 value) { this->write(addr, value); });
	}
}

Serial::~Serial() {}

u8 Serial::read(u16 address)
{
	switch (address) {
	case 0xff01:
		return serial_data;
	case 0xff02:
		return serial_control;
	default:
		return 0xff;
	}
}

void Serial::write(u16 address, u8 value)
{
	switch (address) {
	case 0xff01:
		serial_data = value;
		break;
	case 0xff02:
		serial_control = value;
		if (value == 0x81) {
			std::cout << static_cast<char>(serial_data);
			serial_control = 0x01;
			gameboy.getCPU().requestInterrupt(CPU::Interrupt::SERIAL);
		}
		break;
	}
}
