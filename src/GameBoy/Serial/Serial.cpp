#include <iostream>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/Serial/Serial.hpp>

Serial::Serial(GameBoy &gb) : _gb(gb)
{
#ifndef NDEBUG
	std::cout << "new Serial" << std::endl;
#endif
}

Serial::~Serial()
{
#ifndef NDEBUG
	std::cout << "Serial deleted" << std::endl;
#endif
}
