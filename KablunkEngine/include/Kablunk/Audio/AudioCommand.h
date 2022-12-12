#pragma once
#ifndef KABLUNK_AUDIO_AUDIO_COMMAND_H
#define KABLUNK_AUDIO_AUDIO_COMMAND_H

#include "Kablunk/Audio/AudioCore.h"
#include "Kablunk/Audio/AudioEngine.h"

#include "Kablunk/Asset/AssetCommand.h"

namespace Kablunk::audio
{
	// \brief initialize the audio engine
	inline void init_audio_engine() { Singleton<AudioEngine>::get().init(); }

	// \brief shutdown the audio engine
	inline void shutdown_audio_engine() { Singleton<AudioEngine>::get().shutdown(); }

	// \brief play an "inline" sound
	inline void play_sound_inline(const asset::asset_id_t& sound_id)
	{
		ma_result result = Singleton<AudioEngine>::get().play_sound_inline(sound_id);
		if (result != MA_SUCCESS)
			KB_CORE_ERROR("[AudioEngine]: failed to play sound! error code '{}'", result);
	}

	// \brief add sound to audio queue
	inline void queue_sound(const asset::asset_id_t& sound_id)
	{
		ref<AudioAsset> sound_asset = asset::get_asset<AudioAsset>(sound_id);
		Singleton<AudioEngine>::get().add_to_queue(sound_asset);
	}

	// \brief add sound to audio queue
	inline void queue_sound(ref<AudioAsset>& sound_asset) { Singleton<AudioEngine>::get().add_to_queue(sound_asset); }

	// \brief stop all playing audio and clear the queue
	inline void stop_and_clear_queue() { Singleton<AudioEngine>::get().stop_and_clear_queue(); }

}

#endif
