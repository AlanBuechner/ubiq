#pragma once
#include "Engine/Renderer/Buffer.h"

namespace Engine
{
	class ENGINE_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32 size);
		OpenGLVertexBuffer(float* vertices, uint32 size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
		virtual BufferLayout& GetLayout() override { return m_Layout; };

		virtual void SetData(const void* data, uint32 size) override;

	private:
		uint32 m_RendererID;
		BufferLayout m_Layout;
	};

	class ENGINE_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32 count);
		OpenGLIndexBuffer(uint32* indices, uint32 count);
		virtual ~OpenGLIndexBuffer();

		virtual uint32 GetCount() override { return m_Count; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const uint32* data, uint32 count) override;

	private:
		uint32 m_RendererID;
		uint32 m_Count;
	};
}
