#include <iostream>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/APU/APU.hpp>

APU::APU(GameBoy &gb) : _gb(gb)
{
	std::cout << "new APU" << std::endl;
}

APU::~APU()
{
	std::cout << "APU deleted" << std::endl;
}
