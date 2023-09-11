#include "kablunkpch.h"
#include "Kablunk/Core/Layer.h"

namespace kb {
	Layer::Layer(const std::string& debugName) : m_DebugName(debugName) { }

	Layer::~Layer() {}
}