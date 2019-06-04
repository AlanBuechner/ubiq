#pragma once

namespace Engine
{
	enum class RendererAPI
	{
		None = 0, OpenGl = 1
	};

	class Renderer
	{
	public:
		inline static RendererAPI GetrAPI() { return s_RendererAPI; }

	private:
		static RendererAPI s_RendererAPI;
	};
}