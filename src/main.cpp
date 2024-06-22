#include <iostream>
#include <GameBoy/GameBoy.hpp>

int main()
{
	GameBoy gb("roms/Tetris.gb", "dmg_bios.bin");
	gb.run();

	return 0;
}
