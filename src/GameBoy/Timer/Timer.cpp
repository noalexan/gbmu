#include "Timer.hpp"
#include "../GameBoy.hpp"

Timer::Timer(GameBoy &gb)
    : gameboy(gb), div(registers[0]), tima(registers[1]), tma(registers[2]), tac(registers[3])
{
}

Timer::~Timer() {}

void Timer::tick() {}
