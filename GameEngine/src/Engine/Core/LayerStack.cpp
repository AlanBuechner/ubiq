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
		m_Layers.Insert(m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.Push(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		for (Layer** it = m_Layers.begin(); it != m_Layers.end(); ++it)
		{
			if (layer == *it)
			{
				layer->OnDetach();
				m_Layers.Remove(it);
				m_LayerInsertIndex--;
				break;
			}
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		for (Layer** it = m_Layers.begin(); it != m_Layers.end(); ++it)
		{
			if (overlay == *it)
			{
				overlay->OnDetach();
				m_Layers.Remove(it);
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
		m_Layers.Clear();
	}

}
