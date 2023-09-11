#include "kablunkpch.h"
#include "Kablunk/Core/LayerStack.h"

namespace kb {
	LayerStack::LayerStack() {}

	LayerStack::~LayerStack() 
	{
		Destroy();
	}

	void LayerStack::Destroy()
	{
		for (Layer* layer : m_layers)
		{
			layer->OnDetach();
			delete layer;
		}

		m_layers.clear();
	}

	void LayerStack::PushLayer(Layer* layer) {
		m_layers.emplace(m_layers.begin() + m_LayerInsertIndex++, layer);
	}

	void LayerStack::PushOverlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer) {
		auto it = std::find(m_layers.begin() + m_LayerInsertIndex, m_layers.end(), layer);
		if (it != m_layers.begin() + m_LayerInsertIndex) {
			m_layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay) {
		auto it = std::find(m_layers.begin() + m_LayerInsertIndex, m_layers.end(), overlay);
		if (it != m_layers.end()) {
			m_layers.erase(it);
		}
	}

}
