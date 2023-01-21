#pragma once
#ifndef KABLUNK_AUDIO_AUDIO_ASSET_H
#define KABLUNK_AUDIO_AUDIO_ASSET_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/Asset.h"

#include <miniaudio.h>

namespace Kablunk::audio
{

	struct audio_asset_config
	{
		std::filesystem::path filepath;
	};

	class AudioAsset : public asset::Asset
	{
	public:
		AudioAsset() = default;
		AudioAsset(const audio_asset_config& config);
		~AudioAsset() = default;

		// factory to create audio assets using a passed in config
		static ref<AudioAsset> create(const audio_asset_config& config);
		// get whether the sound was loaded
		bool loaded() const { return m_loaded; }
		// start playing the sound
		void play();
		// stop the sound from playing
		void stop();
		// equality comparison operator
		bool operator==(const AudioAsset& other) const { return get_id() == other.get_id(); }
		// inequality comparison operator
		bool operator!=(const AudioAsset& other) const { return !(*this == other); }
		// check if the audio is playing
		bool is_playing() const { return ma_sound_is_playing(&m_sound); }
		// get a pointer to the underlying ma sound object
		const ma_sound* get_ma_sound() const { return &m_sound; }
		// get a pointer to the underlying ma sound object
		ma_sound* get_ma_sound() { return &m_sound; }
	private:
		// flag for whether the audio asset has finished loading
		bool m_loaded = false;
		// ma sound object
		ma_sound m_sound{};
	};

	using sound_asset_t = AudioAsset;
}

#endif
