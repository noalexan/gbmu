#include <GameBoy.hpp>

int main(int argc, char *argv[])
{
	GameBoy gb(argv[1]);
	gb.run();

	return 0;
}
