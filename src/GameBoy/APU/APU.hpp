#pragma once

#include <SDL2/SDL.h>
#include <types.h>

class GameBoy;

class APU {
private:
	SDL_AudioDeviceID audio_device;

	static void audioCallback(void *userdata, u8 *stream, int len);

	enum NR10 {
		SWEEP_SHIFT     = 0x07,   // Sweep shift (0-7)
		SWEEP_DIRECTION = 1 << 3, // 0=Addition, 1=Subtraction
		SWEEP_TIME_MASK = 0x70    // Sweep time bits
	};

	enum NR11_NR21 {
		LENGTH_TIMER_MASK = 0x3F, // Length timer (0-63)
		DUTY_MASK         = 0xC0  // Duty cycle bits
	};

	enum NR12_NR22_NR42 {
		ENVELOPE_PERIOD    = 0x07,   // Envelope sweep (0-7)
		ENVELOPE_DIRECTION = 1 << 3, // 0=Decrease, 1=Increase
		INITIAL_VOLUME     = 0xF0    // Initial volume bits
	};

	enum NR14_NR24_NR34_NR44 {
		PERIOD_HIGH_MASK = 0x07,   // High 3 bits of period
		LENGTH_ENABLE    = 1 << 6, // Length enable
		TRIGGER          = 1 << 7  // Trigger
	};

	enum NR30 {
		DAC_ENABLE = 1 << 7 // DAC on/off
	};

	enum NR32 {
		OUTPUT_LEVEL_MASK = 0x60, // Output level select (00=mute, 01=100%, 10=50%, 11=25%)
		OUTPUT_LEVEL_100  = 0x20,
		OUTPUT_LEVEL_50   = 0x40,
		OUTPUT_LEVEL_25   = 0x60
	};

	enum NR43 {
		DIVISOR_CODE = 0x07,   // Frequency divisor code
		LFSR_WIDTH   = 1 << 3, // 0=15bit, 1=7bit
		CLOCK_SHIFT  = 0xF0    // Clock shift bits
	};

	enum NR50 {
		RIGHT_VOLUME = 0x07,   // Right volume (0-7)
		VIN_RIGHT    = 1 << 3, // VIN to right enable
		LEFT_VOLUME  = 0x70,   // Left volume bits
		VIN_LEFT     = 1 << 7  // VIN to left enable
	};

	enum NR51 {
		CHANNEL_1_RIGHT = 1 << 0,
		CHANNEL_2_RIGHT = 1 << 1,
		CHANNEL_3_RIGHT = 1 << 2,
		CHANNEL_4_RIGHT = 1 << 3,
		CHANNEL_1_LEFT  = 1 << 4,
		CHANNEL_2_LEFT  = 1 << 5,
		CHANNEL_3_LEFT  = 1 << 6,
		CHANNEL_4_LEFT  = 1 << 7
	};

	enum NR52 {
		CHANNEL_1_ON = 1 << 0,
		CHANNEL_2_ON = 1 << 1,
		CHANNEL_3_ON = 1 << 2,
		CHANNEL_4_ON = 1 << 3,
		AUDIO_ENABLE = 1 << 7
	};

	// Channel 1 - Tone & Sweep
	u8 &nr10; // Sweep
	u8 &nr11; // Length timer & duty cycle
	u8 &nr12; // Volume & envelope
	u8 &nr13; // Period low
	u8 &nr14; // Period high & control

	// Channel 2 - Tone
	u8 &nr21; // Length timer & duty cycle
	u8 &nr22; // Volume & envelope
	u8 &nr23; // Period low
	u8 &nr24; // Period high & control

	// Channel 3 - Wave
	u8 &nr30; // DAC enable
	u8 &nr31; // Length timer
	u8 &nr32; // Output level
	u8 &nr33; // Period low
	u8 &nr34; // Period high & control

	// Channel 4 - Noise
	u8 &nr41; // Length timer
	u8 &nr42; // Volume & envelope
	u8 &nr43; // Frequency & randomness
	u8 &nr44; // Control

	// Control registers
	u8 &nr50; // Master volume & VIN panning
	u8 &nr51; // Sound panning
	u8 &nr52; // Sound on/off

public:
	APU();
	virtual ~APU();

	u8 registers[0x17];
	u8 wave_pattern[0x10];
};
