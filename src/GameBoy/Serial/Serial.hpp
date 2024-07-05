#pragma once

#include <utils/types.hpp>

class GameBoy;

class Serial
{
public:
	Serial(GameBoy &);
	~Serial();

	u8 registers[0x02];

private:
	GameBoy &_gb;
};
