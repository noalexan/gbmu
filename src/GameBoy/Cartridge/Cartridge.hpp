#pragma once

#include <cstddef>
#include <string>
#include <types.h>

class Cartridge {
private:
	u8    *rom_data;
	size_t rom_size;

	static const std::string CARTRIDGE_TYPES[256];

public:
	Cartridge(const std::string &filename);
	virtual ~Cartridge();

	std::string getTitle() const;
	u8          getCartridgeType() const;
	std::string getCartridgeTypeString() const;
	u8          getRomSize() const;
	u8          getRamSize() const;
	u8          getLicenseCode() const;
	u8          getHeaderChecksum() const;
	u16         getGlobalChecksum() const;
	const u8   *getRomData() const;
	size_t      getRomDataSize() const;
};
