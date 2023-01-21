#include "kablunkpch.h"
#include "Kablunk/Audio/AudioAsset.h"

#include "Kablunk/Audio/AudioEngine.h"

namespace Kablunk::audio
{
	AudioAsset::AudioAsset(const audio_asset_config& config)
	{
		ma_uint32 flags = (AudioEngine::get().get_config().async_ma_audio ? MA_SOUND_FLAG_ASYNC : 0) | MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION;
		ma_result result = ma_sound_init_from_file(AudioEngine::get().get_ma_engine(), config.filepath.string().c_str(), flags, nullptr, nullptr, &m_sound);
		if (result != MA_SUCCESS)
		{
			KB_CORE_ERROR("[AudioAsset]: miniaudio engine failed to initialize sound from file '{}'", config.filepath);
			m_loaded = false;
		}
		else
			m_loaded = true;
	}

	ref<AudioAsset> AudioAsset::create(const audio_asset_config& config)
	{
		return IntrusiveRef<AudioAsset>::Create(config);
	}

	void AudioAsset::play()
	{
		if (!ma_sound_is_playing(&m_sound))
			ma_sound_start(&m_sound);
	}

	void AudioAsset::stop()
	{
		if (!ma_sound_is_playing(&m_sound))
			return;

		ma_result result = ma_sound_stop(&m_sound);
		if (result != MA_SUCCESS)
			KB_CORE_ERROR("[AudioAsset]: failed to stop sound! error code '{}'", result);
	}

}
