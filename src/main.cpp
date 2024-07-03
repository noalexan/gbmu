#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GameBoy/GameBoy.hpp>

static std::vector<u8> load_file(const char *path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return std::vector<u8>();
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<u8> buffer(size);
	if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
	{
		std::stringstream os;

		os << "Failed to read file: " << path << std::endl;
		throw std::runtime_error(os.str().c_str());
	}

	return buffer;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <bios> <rom>" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::vector<u8> bios = load_file(argv[1]), rom = load_file(argv[2]);

	GameBoy gb(bios, rom);
	gb.run();

	return 0;
}
