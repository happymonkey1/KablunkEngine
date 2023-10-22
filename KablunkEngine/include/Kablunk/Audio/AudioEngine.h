#pragma once
#ifndef KABLUNK_AUDIO_AUDIO_ENGINE_H
#define KABLUNK_AUDIO_AUDIO_ENGINE_H

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Audio/AudioAsset.h"


#pragma warning(push, 0)
#include <miniaudio.h>
#pragma warning(pop)

#include <vector>

namespace kb::audio
{

	struct audio_engine_config
	{
		// master audio for the whole audio engine
		float master_volume = 0.8f;

		// flag for whether we let miniaudio asynchronously load sound files
		bool async_ma_audio = false;
	};

	class AudioEngine
	{
	public:
		// initialize the audio engine
		void init();
		// shutdown the audio engine
		void shutdown();
		// start the ma audio engine
		void start_engine();
		// stop the ma audio engine
		void stop_engine();
		// play an "inline" sound, which is played once, then registered to be recycled.
		// #TODO create wrapper for miniaudio error
		ma_result play_sound_inline(const asset::asset_id_t& sound_asset_id);
		// get the underyling ma_engine object
		ma_engine* get_ma_engine() { return &m_engine; }
		// get the underlying ma_engine object
		const ma_engine* get_ma_engine() const { return &m_engine; }
		// queue audio to be played by the engine
		void add_to_queue(ref<AudioAsset>& audio_asset, bool autoplay = false);
		// stop all audio sources in the queue and clear
		void stop_and_clear_queue();
		// get singleton of the audio engine
		SINGLETON_GET_FUNC(AudioEngine);
		// get the audio engine configuration
		const audio_engine_config& get_config() const { return m_engine_config; }
		// get the master volume currently set
		float get_master_volume() const { return m_engine_config.master_volume;}
		// set the master volume
		void set_master_volume(float new_volume);
		// get the audio queue
		const std::vector<ref<AudioAsset>>& get_audio_queue() const { return m_queued_audio; }
		// get the audio queue
		std::vector<ref<AudioAsset>>& get_audio_queue() { return m_queued_audio; }
	private:
		// settings for the audio engine
		audio_engine_config m_engine_config{};
		// configuration for a miniaudio device
		ma_device_config m_ma_device_config;
		// miniaudio device
		ma_device m_device;
		// miniaudio high-level audio engine
		ma_engine m_engine;
		// queue of audio assets being played
		std::vector<ref<AudioAsset>> m_queued_audio;
	};

}

#endif
