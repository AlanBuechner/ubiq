#pragma once

namespace Engine
{
	class Descriptor
	{
	public:
		virtual ~Descriptor() = 0;

		virtual void Bind() = 0;
	};
}
