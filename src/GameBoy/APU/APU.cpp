#include "APU.hpp"
#include <cmath>

void APU::audioCallback(void *userdata, u8 *stream, int len)
{
	APU   *apu     = static_cast<APU *>(userdata);
	float *samples = reinterpret_cast<float *>(stream);
	int    count   = len / sizeof(float);

	float phase_increment = apu->frequency / apu->sample_rate;

	for (int i = 0; i < count; i++) {
		float value = (apu->phase < 0.5f) ? 0.2f : -0.2f;
		samples[i]  = value;

		apu->phase += phase_increment;
		if (apu->phase >= 1.0f) {
			apu->phase -= 1.0f;
		}
	}
}

APU::APU()
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec want, have;
	SDL_memset(&want, 0, sizeof(want));

	want.freq     = static_cast<int>(sample_rate);
	want.format   = AUDIO_F32SYS;
	want.channels = 1;
	want.samples  = 512;
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
