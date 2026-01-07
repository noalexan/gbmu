#pragma once

#include <cstdint>
#include <types.h>

class GameBoy;

class Serial {
private:
	GameBoy &gameboy;

	u8 serial_data    = 0;
	u8 serial_control = 0;

public:
	Serial(GameBoy &);
	virtual ~Serial();

	u8   read(u16 address);
	void write(u16 address, u8 value);

	u8 &getSerialData() { return serial_data; }
	u8 &getSerialControl() { return serial_control; }
};
