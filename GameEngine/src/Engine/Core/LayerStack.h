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
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		void RemoveAllLayers();

		Layer** begin() { return &*m_Layers.begin(); }
		Layer** end() { return &*m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};

}
