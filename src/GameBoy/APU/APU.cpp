#include <iostream>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/APU/APU.hpp>

APU::APU(GameBoy &gb) : _gb(gb)
{
#ifndef NDEBUG
	std::cout << "new APU" << std::endl;
#endif
}

APU::~APU()
{
#ifndef NDEBUG
	std::cout << "APU deleted" << std::endl;
#endif
}
