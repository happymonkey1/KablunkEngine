#include "kablunkpch.h"
#include "Kablunk/Audio/AudioEngine.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/AssetCommand.h"

#include <glm/glm.hpp>

namespace kb::audio
{
	// documentation for miniaudio https://miniaud.io/docs/manual/index.html

	// In playback mode copy data to output. In capture mode read data from input. In full-duplex mode, both
	// output and input will be valid and you can move data from input into output. Never process more than
	// frame_count frames.
	void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count)
	{

	}

	void AudioEngine::init()
	{
		m_ma_device_config = ma_device_config_init(ma_device_type_playback);
		m_ma_device_config.playback.format = ma_format_s32;   // set to ma_format_unknown to use the device's native format
		m_ma_device_config.playback.channels = 2;				// set to 0 to use the device's native channel count
		m_ma_device_config.sampleRate = 48000;				// set to 0 to use the device's native sample rate
		m_ma_device_config.dataCallback = data_callback;		// function that is called when miniaudio needs more data
		m_ma_device_config.pUserData = nullptr;				// can be access from the device object

		if (ma_device_init(nullptr, &m_ma_device_config, &m_device) != MA_SUCCESS)
			KB_CORE_ASSERT(false, "failed to initialize audio device!");

		ma_device_start(&m_device);

		// configure and initialize engine
		ma_engine_config engine_config = ma_engine_config_init();
		engine_config.noAutoStart = false;
		engine_config.noDevice = 1;
		engine_config.channels = 2;
		engine_config.sampleRate = 48000;

		if (ma_engine_init(&engine_config, &m_engine) != MA_SUCCESS)
			KB_CORE_ASSERT(false, "failed to initialize audio engine!");

		KB_CORE_INFO("[AudioEngine]: initialized.");
	}

	void AudioEngine::shutdown()
	{
		// shutdown the device(s)
		ma_device_uninit(&m_device);
		// shutdown the engine
		ma_engine_uninit(&m_engine);
		// clear all queued audio assets
		m_queued_audio.clear();

		KB_CORE_INFO("[AudioEngine]: shut down.");
	}

	ma_result AudioEngine::play_sound_inline(const asset::asset_id_t& sound_asset_id)
	{
		std::filesystem::path sound_filepath = asset::get_absolute_path(sound_asset_id);
		if (sound_filepath.empty())
		{
			KB_CORE_ERROR("[AudioEngine]: tried playing inline sound '{}' but it was not a valid asset!", sound_filepath);
			return MA_ERROR;
		}

		std::string audio_filepath_str = sound_filepath.string();
		const char* sound_cstr = audio_filepath_str.c_str();

		return ma_engine_play_sound(&m_engine, sound_cstr, nullptr);
	}

	void AudioEngine::set_master_volume(float new_volume)
	{
		// clamp volume between 0.0 and 2.0
		// volume > 1.0 in miniaudio results in amplification
		glm::clamp(new_volume, 0.0f, 2.0f);
		m_engine_config.master_volume = new_volume;
		ma_engine_set_volume(&m_engine, new_volume);
	}

	void AudioEngine::start_engine()
	{
		ma_engine_start(&m_engine);
	}

	void AudioEngine::stop_engine()
	{
		ma_engine_stop(&m_engine);
	}

	void AudioEngine::add_to_queue(ref<AudioAsset>& audio_asset, bool autoplay /*= false*/)
	{
		KB_CORE_ASSERT(audio_asset, "trying to queue null audio asset!");

		if (std::find(m_queued_audio.begin(), m_queued_audio.end(), audio_asset) != m_queued_audio.end())
		{
			const asset::AssetMetadata& metadata = asset::try_get_asset_metadata(audio_asset->get_id());
			KB_CORE_WARN("[AudioEngine]: audio '{}' is already in the play queue.", metadata.filepath);
		}

		m_queued_audio.push_back(audio_asset);

		// #TODO autoplay should be on the audio source
		if (autoplay)
			audio_asset->play();
	}

	void AudioEngine::stop_and_clear_queue()
	{
		for (auto& audio : m_queued_audio)
			audio->stop();

		m_queued_audio.clear();
	}

}
