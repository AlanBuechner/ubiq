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

		struct FBTransitionObject
		{
			Ref<FrameBuffer> fb;
			FrameBufferState to, from;
		};

	public:

		virtual void SignalRecording() = 0;
		virtual void StartRecording(Ref<ShaderPass> startShader = nullptr) = 0;

		// transitions
		void ToRenderTarget(Ref<FrameBuffer> fb, FrameBufferState from) { Transition({ fb }, FrameBufferState::RenderTarget, from); }
		void ToSRV(Ref<FrameBuffer> fb, FrameBufferState from) { Transition({ fb }, FrameBufferState::SRV, from); }
		void Present(FrameBufferState from) { Present(nullptr, from); }
		virtual void Present(Ref<FrameBuffer> fb, FrameBufferState from) = 0;

		virtual void Transition(std::vector<Ref<FrameBuffer>> fbs, FrameBufferState to, FrameBufferState from) = 0;
		virtual void Transition(std::vector<FBTransitionObject> transitions) = 0;

		// rendering
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) = 0;

		virtual void ClearRenderTarget() = 0;
		virtual void ClearRenderTarget(uint32 attachment) = 0;
		virtual void ClearRenderTarget(uint32 attachment, const Math::Vector4& color) = 0;

		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer) = 0;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment) = 0;
		virtual void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color) = 0;

		virtual void SetShader(Ref<ShaderPass> shader) = 0;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) = 0;
		virtual void SetRootConstant(uint32 index, uint32 data) = 0;
		virtual void SetTexture(uint32 index, Ref<Texture> texture) = 0;
		virtual void SetFrameBuffer(uint32 index, Ref<FrameBuffer> buffer, uint32 attatchment) = 0;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) = 0;
		virtual void ExecuteBundle(Ref<CommandList> commandList) = 0;

		// compute
		virtual void SetComputeShader(Ref<ComputeShader> shader) = 0;

		virtual void Close() = 0;

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);
	};
}
