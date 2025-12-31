#pragma once

#include <types.h>

class GameBoy;

class APU {
public:
	APU();
	virtual ~APU();

	u8 registers[0x17];
};
