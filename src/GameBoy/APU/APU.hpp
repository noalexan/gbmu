#pragma once

#include <SDL2/SDL.h>
#include <types.h>

class GameBoy;

class APU {
private:
	SDL_AudioDeviceID audio_device = 0;
	float             phase        = 0.0f;
	float             frequency    = 440.0f;
	float             sample_rate  = 44100.0f;

	static void audioCallback(void *userdata, u8 *stream, int len);

public:
	APU();
	virtual ~APU();

	u8 registers[0x17];
};
