#include <GBMU/GameBoy.hpp>

int main(int argc, char *argv[])
{
	GBMU::GameBoy gb(argv[1]);
	gb.run();

	return 0;
}
