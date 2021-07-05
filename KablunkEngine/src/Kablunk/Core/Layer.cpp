#include "kablunkpch.h"
#include "Kablunk/Core/Layer.h"

namespace Kablunk {
	Layer::Layer(const std::string& debugName) : m_DebugName(debugName) { }

	Layer::~Layer() {}
}