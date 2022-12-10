#pragma once
#ifndef KABLUNK_AUDIO_AUDIO_COMMAND_H
#define KABLUNK_AUDIO_AUDIO_COMMAND_H

#include "Kablunk/Audio/AudioCore.h"
#include "Kablunk/Audio/AudioEngine.h"

namespace Kablunk::audio
{
	// initialize the audio engine
	inline void init_audio_engine()
	{
		Singleton<AudioEngine>::get().init();
	}

	// shutdown the audio engine
	inline void shutdown_audio_engine()
	{
		Singleton<AudioEngine>::get().shutdown();
	}

	// play an "inline" sound
	inline void play_sound_inline(const asset::asset_id_t& sound_id)
	{
		ma_result result = Singleton<AudioEngine>::get().play_sound_inline(sound_id);
		if (result != MA_SUCCESS)
			KB_CORE_ERROR("[AudioEngine]: failed to play sound! error code '{}'", result);
	}

}

#endif
