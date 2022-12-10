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

	private:
		// flag for whether the audio asset has finished loading
		bool m_loaded = false;
		// sound object
		ma_sound m_sound{};
	};

	using sound_asset_t = AudioAsset;
}

#endif
