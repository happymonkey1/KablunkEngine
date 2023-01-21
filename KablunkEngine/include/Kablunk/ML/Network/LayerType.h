#pragma once
#ifndef KABLUNK_ML_NETWORK_LAYER_TYPE_H
#define KABLUNK_ML_NETWORK_LAYER_TYPE_H

namespace Kablunk::ml::network
{

	enum class LayerType
	{
		Input = 0,
		Hidden,
		Output,
		NONE
	};

	using layer_type_t = LayerType;

}

#endif
