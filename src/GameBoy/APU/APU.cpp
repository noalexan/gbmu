#include "APU.hpp"
#include <cmath>

void APU::audioCallback(void *userdata, u8 *stream, int len)
{
	APU *apu = static_cast<APU *>(userdata);

	static float ch1_phase            = 0.0f;
	static int   ch1_length_counter   = 0;
	static int   ch1_envelope_counter = 0;
	static int   ch1_envelope_volume  = 0;
	static int   ch1_sweep_counter    = 0;
	static u16   ch1_shadow_frequency = 0;
	static bool  ch1_sweep_enabled    = false;

	SDL_memset(stream, 0, len);

	if (!(apu->nr52 & AUDIO_ENABLE)) {
		return;
	}

	s16 *samples = reinterpret_cast<s16 *>(stream);
	int  count   = len / sizeof(s16) / 2;

	const float left_volume  = (float)(((apu->nr50 & LEFT_VOLUME) >> 4) + 1) / 8.0f;
	const float right_volume = (float)((apu->nr50 & RIGHT_VOLUME) + 1) / 8.0f;

	static const float DUTY_CYCLES[4] = {0.125f, 0.25f, 0.5f, 0.75f};

	const int SAMPLES_PER_FRAME_STEP = 44100 / 512;

	if (apu->nr14 & TRIGGER) {
		ch1_phase = 0.0f;
		apu->nr52 |= CHANNEL_1_ON;
		apu->nr14 &= ~TRIGGER;

		if (apu->nr14 & LENGTH_ENABLE) {
			int length_load    = apu->nr11 & LENGTH_TIMER_MASK;
			ch1_length_counter = (64 - length_load) * (44100 / 256);
		} else {
			ch1_length_counter = 0;
		}

		ch1_envelope_volume  = (apu->nr12 & INITIAL_VOLUME) >> 4;
		int envelope_period  = apu->nr12 & ENVELOPE_PERIOD;
		ch1_envelope_counter = envelope_period * (44100 / 64);

		u16 period           = ((apu->nr14 & PERIOD_HIGH_MASK) << 8) | apu->nr13;
		ch1_shadow_frequency = period;
		int sweep_period     = (apu->nr10 & SWEEP_TIME_MASK) >> 4;
		int sweep_shift      = apu->nr10 & SWEEP_SHIFT;
		ch1_sweep_counter    = sweep_period * SAMPLES_PER_FRAME_STEP;
		ch1_sweep_enabled    = (sweep_period != 0 || sweep_shift != 0);

		if ((apu->nr12 & 0xF8) == 0) {
			apu->nr52 &= ~CHANNEL_1_ON;
		}
	}

	if (apu->nr52 & CHANNEL_1_ON) {
		u16   period         = ((apu->nr14 & PERIOD_HIGH_MASK) << 8) | apu->nr13;
		float frequency      = 131072.0f / (float)(2048 - period);
		float duty_threshold = DUTY_CYCLES[(apu->nr11 >> 6) & 0x03];
		float period_samples = 44100.0f / frequency;

		int envelope_period = apu->nr12 & ENVELOPE_PERIOD;
		int sweep_period    = (apu->nr10 & SWEEP_TIME_MASK) >> 4;
		int sweep_shift     = apu->nr10 & SWEEP_SHIFT;

		for (int i = 0; i < count; i++) {
			if ((apu->nr14 & LENGTH_ENABLE) && ch1_length_counter > 0) {
				ch1_length_counter--;
				if (ch1_length_counter <= 0) {
					apu->nr52 &= ~CHANNEL_1_ON;
					break;
				}
			}

			if (envelope_period != 0 && ch1_envelope_counter > 0) {
				ch1_envelope_counter--;
				if (ch1_envelope_counter <= 0) {
					if (apu->nr12 & ENVELOPE_DIRECTION) {
						if (ch1_envelope_volume < 15) {
							ch1_envelope_volume++;
						}
					} else {
						if (ch1_envelope_volume > 0) {
							ch1_envelope_volume--;
						}
					}
					ch1_envelope_counter = envelope_period * (44100 / 64);
				}
			}

			if (ch1_sweep_enabled && sweep_period != 0 && ch1_sweep_counter > 0) {
				ch1_sweep_counter--;
				if (ch1_sweep_counter <= 0) {
					u16 new_period;
					u16 delta = ch1_shadow_frequency >> sweep_shift;

					if (apu->nr10 & SWEEP_DIRECTION) {
						new_period = ch1_shadow_frequency - delta;
					} else {
						new_period = ch1_shadow_frequency + delta;
					}

					if (new_period > 2047) {
						apu->nr52 &= ~CHANNEL_1_ON;
						break;
					}

					if (sweep_shift != 0) {
						ch1_shadow_frequency = new_period;
						apu->nr13            = new_period & 0xFF;
						apu->nr14            = (apu->nr14 & 0xF8) | ((new_period >> 8) & 0x07);

						period         = new_period;
						frequency      = 131072.0f / (float)(2048 - period);
						period_samples = 44100.0f / frequency;
					}

					ch1_sweep_counter = sweep_period * SAMPLES_PER_FRAME_STEP;
				}
			}

			float volume = (float)ch1_envelope_volume / 15.0f;
			float wave   = (ch1_phase < duty_threshold) ? 1.0f : -1.0f;
			s16   sample = (s16)(wave * volume * 4096.0f);

			if (apu->nr51 & CHANNEL_1_LEFT)
				samples[i * 2] += (s16)(sample * left_volume);
			if (apu->nr51 & CHANNEL_1_RIGHT)
				samples[i * 2 + 1] += (s16)(sample * right_volume);

			ch1_phase += 1.0f / period_samples;
			if (ch1_phase >= 1.0f)
				ch1_phase -= 1.0f;
		}
	}
}

APU::APU()
    : nr10(registers[0x00]), nr11(registers[0x01]), nr12(registers[0x02]), nr13(registers[0x03]),
      nr14(registers[0x04]), nr21(registers[0x06]), nr22(registers[0x07]), nr23(registers[0x08]),
      nr24(registers[0x09]), nr30(registers[0x0A]), nr31(registers[0x0B]), nr32(registers[0x0C]),
      nr33(registers[0x0D]), nr34(registers[0x0E]), nr41(registers[0x10]), nr42(registers[0x11]),
      nr43(registers[0x12]), nr44(registers[0x13]), nr50(registers[0x14]), nr51(registers[0x15]),
      nr52(registers[0x16])
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec want, have;
	SDL_memset(&want, 0, sizeof(want));

	want.freq     = 44100;
	want.format   = AUDIO_S16SYS;
	want.channels = 2;
	want.samples  = 1024;
	want.callback = audioCallback;
	want.userdata = this;

	audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
	if (audio_device != 0) {
		SDL_PauseAudioDevice(audio_device, 0);
	}
}

APU::~APU()
{
	if (audio_device != 0) {
		SDL_CloseAudioDevice(audio_device);
	}
}
