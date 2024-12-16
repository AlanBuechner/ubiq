#include "pch.h"
#include "LayerStack.h"
#include "Layer.h"

namespace Engine {

	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		RemoveAllLayers();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.insert(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.push_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it)
		{
			if (layer == *it)
			{
				layer->OnDetach();
				m_Layers.erase(it);
				m_LayerInsertIndex--;
				break;
			}
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it)
		{
			if (overlay == *it)
			{
				overlay->OnDetach();
				m_Layers.erase(it);
				break;
			}
		}
	}

	void LayerStack::RemoveAllLayers()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
		m_Layers.clear();
	}

}
