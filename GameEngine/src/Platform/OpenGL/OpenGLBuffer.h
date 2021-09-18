#pragma once
#include "Engine/Renderer/Buffer.h"

namespace Engine
{
	class ENGINE_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
		virtual BufferLayout& GetLayout() override { return m_Layout; };

		virtual void SetData(const void* data, uint32_t size) override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class ENGINE_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count);
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual uint32_t GetCount() override { return m_Count; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const uint32_t* data, uint32_t count) override;

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}