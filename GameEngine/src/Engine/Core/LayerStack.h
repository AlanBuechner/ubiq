#pragma once

#include "Engine/Core/Core.h"



namespace Engine
{
	class Layer;
}

namespace Engine {

	class  LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void RemoveLayer(Layer* layer);

		void RemoveLayers();
		void RemoveOverlays();
		void Clear();

		void Destroy();

		void UpdateLayerChanges();

		Layer** begin() { return &*m_Layers.begin(); }
		Layer** end() { return &*m_Layers.end(); }
	private:
		Utils::Vector<Layer*> m_Layers;
		Utils::Vector<Layer*> m_LayersToAdd;
		Utils::Vector<Layer*> m_OverlaysToAdd;
		Utils::Vector<Layer*> m_LayersToRemove;
		bool m_Invalidate = false;
		unsigned int m_LayerInsertIndex = 0;
	};

}
