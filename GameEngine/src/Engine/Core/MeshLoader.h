#pragma once
#include "Core.h"
#include "Mesh.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Engine
{
	enum class VertexDataType
	{
		None = 0,
		Position3, Position4,
		UV,
		Normal,
		Tangent,
	};

	struct VertexElement
	{
		VertexDataType type;
		uint32 Offset;
		uint32 Size;

		VertexElement(VertexDataType type) :
			type(type)
		{
			CalculateSize();
		}

	private:
		inline void CalculateSize()
		{
			switch (type)
			{
			case Engine::VertexDataType::None:
				Size = 0;
				break;
			case Engine::VertexDataType::Tangent:
			case Engine::VertexDataType::Normal:
			case Engine::VertexDataType::Position3:
				Size = sizeof(float) * 3;
				break;
			case Engine::VertexDataType::Position4:
				Size = sizeof(float) * 4;
				break;
			case Engine::VertexDataType::UV:
				Size = sizeof(float) * 2;
				break;
			default:
				break;
			}
		}
	};

	struct VertexLayout
	{
		VertexLayout() {}
		VertexLayout(const std::initializer_list<VertexElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline const std::vector<VertexElement>& GetElements() const { return m_Elements; }
		inline const uint32 GetStride() const { return m_Stride; }

		inline std::vector<VertexElement>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<VertexElement>::iterator end() { return m_Elements.end(); }
		inline std::vector<VertexElement>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<VertexElement>::const_iterator end() const { return m_Elements.end(); }

		inline VertexElement FindElement(VertexDataType type)
		{
			for (auto e : m_Elements)
				if (e.type == type)
					return e;

			return VertexDataType();
		}

		inline bool HasElement(VertexDataType type)
		{
			return FindElement(type).type != VertexDataType::None;
		}

	private:
		void CalculateOffsetsAndStride()
		{
			uint32 offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
			}
			m_Stride = offset;
		}

	private:
		std::vector<VertexElement> m_Elements;
		uint32 m_Stride = 0;
	};

	class MeshLoader
	{
	public:

		static Ref<Mesh> LoadStaticMesh(std::string file, VertexLayout layout);
	};
}
