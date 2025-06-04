#include "pch.h"
#include "LayerStack.h"
#include "Layer.h"

namespace Engine {

	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		Destroy();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_LayersToAdd.Push(layer);
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_OverlaysToAdd.Push(overlay);
	}

	void LayerStack::RemoveLayer(Layer* layer)
	{
		m_LayersToRemove.Push(layer);
	}

	void LayerStack::RemoveLayers()
	{
		for (uint32 i = 0; i < m_LayerInsertIndex; i++)
			m_LayersToRemove.Push(m_Layers[i]);
	}

	void LayerStack::RemoveOverlays()
	{
		for (uint32 i = m_LayerInsertIndex; i < m_Layers.Count(); i++)
			m_LayersToRemove.Push(m_Layers[i]);
	}

	void LayerStack::Clear()
	{
		for (uint32 i = 0; i < m_Layers.Count(); i++)
			m_LayersToRemove.Push(m_Layers[i]);
	}

	void LayerStack::Destroy()
	{
		Clear(); // clear all existing layers

		// delete all layers and overlays to be added
		for (uint32 i = 0; i < m_LayersToAdd.Count(); i++)
			delete m_LayersToAdd[i];
		for (uint32 i = 0; i < m_OverlaysToAdd.Count(); i++)
			delete m_OverlaysToAdd[i];

		// update layer stack with new changes
		UpdateLayerChanges();
	}

	void LayerStack::UpdateLayerChanges()
	{
		CREATE_PROFILE_FUNCTIONI();
		// remove layers
		for (Layer* layer : m_LayersToRemove)
		{
			uint32 layerIndex = m_Layers.Find(layer);
			if(layerIndex == m_Layers.Count()) // could not find
				continue;

			layer->OnDetach();
			if(layer->IsLayerStackOwner())
				delete layer;
			m_Layers.Remove(layerIndex);

			// decrement layer insert index if layer was not an overlay
			if (layerIndex < m_LayerInsertIndex)
				m_LayerInsertIndex--;
		}
		m_LayersToRemove.Clear();

		// add layers
		for (Layer* layer : m_LayersToAdd)
		{
			m_Layers.Insert(m_LayerInsertIndex++, layer);
			layer->OnAttach();
		}
		m_LayersToAdd.Clear();

		// add overlays
		for (Layer* layer : m_OverlaysToAdd)
		{
			m_Layers.Push(layer);
			layer->OnAttach();
		}
		m_OverlaysToAdd.Clear();
	}

}
