#pragma once
#ifndef KABLUNK_AUDIO_AUDIO_ENGINE_H
#define KABLUNK_AUDIO_AUDIO_ENGINE_H

#include "Kablunk/Audio/AudioCore.h"
#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Asset/Asset.h"

#include <miniaudio.h>

namespace Kablunk::audio
{

	class AudioEngine
	{
	public:
		// initialize the audio engine
		void init();
		// shutdown the audio engine
		void shutdown();

		// play an "inline" sound, which is played once, then registered to be recycled.
		// #TODO create wrapper for miniaudio error
		ma_result play_sound_inline(const asset::asset_id_t& sound_asset_id);

		// get singleton of the audio engine
		SINGLETON_GET_FUNC(AudioEngine);

	private:

	private:
		// configuration for a miniaudio device
		ma_device_config m_config;
		// miniaudio device
		ma_device m_device;
		// miniaudio high-level audio engine
		ma_engine m_engine;
	};

}

#endif
