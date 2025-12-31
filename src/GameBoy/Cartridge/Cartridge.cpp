#include "Cartridge.hpp"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <types.h>
#include <unistd.h>

const std::string Cartridge::CARTRIDGE_TYPES[256] = {"ROM ONLY",
                                                     "MBC1",
                                                     "MBC1+RAM",
                                                     "MBC1+RAM+BATTERY",
                                                     "MBC2",
                                                     "MBC2+BATTERY",
                                                     "ROM+RAM",
                                                     "ROM+RAM+BATTERY",
                                                     "MMM01",
                                                     "MMM01+RAM",
                                                     "MMM01+RAM+BATTERY",
                                                     "MBC3+TIMER+BATTERY",
                                                     "MBC3+TIMER+RAM+BATTERY",
                                                     "MBC3",
                                                     "MBC3+RAM",
                                                     "MBC3+RAM+BATTERY",
                                                     "MBC5",
                                                     "MBC5+RAM",
                                                     "MBC5+RAM+BATTERY",
                                                     "MBC5+RUMBLE",
                                                     "MBC5+RUMBLE+RAM",
                                                     "MBC5+RUMBLE+RAM+BATTERY",
                                                     "MBC6",
                                                     "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
                                                     "MBC8",
                                                     "MBC8+RAM",
                                                     "MBC8+RAM+BATTERY",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "",
                                                     "POCKET CAMERA",
                                                     "BANDAI TAMA5",
                                                     "HuC3",
                                                     "HuC1+RAM+BATTERY"};

Cartridge::Cartridge(const std::string &filename) : rom_data(nullptr), rom_size(0)
{
	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		throw std::runtime_error(strerror(errno));
	}

	struct stat sb;
	if (fstat(fd, &sb) < 0) {
		close(fd);
		throw std::runtime_error(strerror(errno));
	}

	u8 *mapped = (u8 *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mapped == MAP_FAILED) {
		close(fd);
		throw std::runtime_error(strerror(errno));
	}

	rom_size = sb.st_size;
	rom_data = new u8[rom_size];
	memcpy(rom_data, mapped, rom_size);
	munmap(mapped, rom_size);

	close(fd);
}

Cartridge::~Cartridge()
{
	if (rom_data != nullptr) {
		delete[] rom_data;
	}
}

std::string Cartridge::getTitle() const
{
	if (rom_size < 0x144 || rom_data == nullptr)
		return "";
	std::string result;
	for (int i = 0; i < 16; i++) {
		u8 ch = rom_data[0x134 + i];
		if (ch == 0)
			break;
		result += (char)ch;
	}
	while (!result.empty() && result.back() == ' ')
		result.pop_back();
	return result;
}

u8 Cartridge::getCartridgeType() const
{
	if (rom_size < 0x148)
		return 0;
	return rom_data[0x147];
}

std::string Cartridge::getCartridgeTypeString() const
{
	return CARTRIDGE_TYPES[getCartridgeType()];
}

u8 Cartridge::getRomSize() const
{
	if (rom_size < 0x149)
		return 0;
	return rom_data[0x148];
}

u8 Cartridge::getRamSize() const
{
	if (rom_size < 0x14A)
		return 0;
	return rom_data[0x149];
}

u8 Cartridge::getLicenseCode() const
{
	if (rom_size < 0x14C)
		return 0;
	return rom_data[0x14B];
}

u8 Cartridge::getHeaderChecksum() const
{
	if (rom_size < 0x14E)
		return 0;
	return rom_data[0x14D];
}

u16 Cartridge::getGlobalChecksum() const
{
	if (rom_size < 0x150)
		return 0;
	return (rom_data[0x14E] << 8) | rom_data[0x14F];
}

u8 *Cartridge::getRomData() { return rom_data; }

const u8 *Cartridge::getRomData() const { return rom_data; }

size_t Cartridge::getRomDataSize() const { return rom_size; }
