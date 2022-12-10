#include "kablunkpch.h"
#include "Kablunk/Audio/AudioEngine.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/AssetCommand.h"

namespace Kablunk::audio
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
		m_config = ma_device_config_init(ma_device_type_playback);
		m_config.playback.format = ma_format_s32;   // set to ma_format_unknown to use the device's native format
		m_config.playback.channels = 2;				// set to 0 to use the device's native channel count
		m_config.sampleRate = 48000;				// set to 0 to use the device's native sample rate
		m_config.dataCallback = data_callback;		// function that is called when miniaudio needs more data
		m_config.pUserData = nullptr;				// can be access from the device object

		if (ma_device_init(nullptr, &m_config, &m_device) != MA_SUCCESS)
			KB_CORE_ASSERT(false, "failed to initialize audio device!");

		ma_device_start(&m_device);

		if (ma_engine_init(nullptr, &m_engine) != MA_SUCCESS)
			KB_CORE_ASSERT(false, "failed to initialize audio engine!");

		KB_CORE_INFO("[AudioEngine]: initialized.");
	}

	void AudioEngine::shutdown()
	{
		ma_device_uninit(&m_device);

		ma_engine_uninit(&m_engine);

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
	

}
