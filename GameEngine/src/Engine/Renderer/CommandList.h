#pragma once
#include "Engine/Core/Core.h"
#include "FrameBuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "InstanceBuffer.h"

namespace Engine
{
	class CommandList
	{
	public:

		enum CommandListType
		{
			Direct,
			Bundle
		};

	public:
		uint32 m_DependencyCount = 0;

	public:

		virtual void StartRecording(Ref<ShaderPass> startShader = nullptr) = 0;
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) = 0;

		virtual void ClearRenderTarget() = 0;
		virtual void ClearRenderTarget(uint32 attachment) = 0;
		virtual void ClearRenderTarget(uint32 attachment, const Math::Vector4& color) = 0;

		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer) = 0;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment) = 0;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color) = 0;

		virtual void SetShader(Ref<ShaderPass> shader) = 0;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) = 0;
		virtual void SetTexture(uint32 index, Ref<Texture> texture) = 0;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer, int numInstances = -1) = 0;
		virtual void Present() = 0;
		virtual void ExecuteBundle(Ref<CommandList> commandList) = 0;

		virtual void Close() = 0;

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);
	};
}
