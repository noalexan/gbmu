#pragma once

#include <cstddef>
#include <string>
#include <types.h>

class Cartridge {
private:
	u8                      *rom_data;
	size_t                   rom_size;
	u8                       ram[0x8000];

	u8                       rom_bank     = 1;
	u8                       ram_bank     = 0;
	bool                     ram_enabled  = false;
	u8                       banking_mode = 0;

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
	size_t      getRomDataSize() const;

	u8         *getRomData();
	const u8   *getRomData() const;

	u8          read_byte(u16 address);
	void        write_byte(u16 address, u8 value);
};
