#include "kablunkpch.h"
#include "Kablunk/Audio/AudioAsset.h"

namespace Kablunk::audio
{
	AudioAsset::AudioAsset(const audio_asset_config& config)
	{
		m_loaded = true;
	}

	ref<AudioAsset> AudioAsset::create(const audio_asset_config& config)
	{
		return IntrusiveRef<AudioAsset>::Create(config);
	}

	

}
