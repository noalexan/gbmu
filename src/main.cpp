#include "GameBoy/GameBoy.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	try {
		GameBoy gb((argc == 2) ? argv[1] : "../roms/Tetris.gb");
		gb.run();
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
