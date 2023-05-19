#pragma once
#include "Engine/Core/Core.h"
#include "Resources/FrameBuffer.h"
#include "Resources/ResourceState.h"

namespace Engine
{
	class Mesh;
	class Shader;
	class ShaderPass;
	class ComputeShader;
	class ConstantBuffer;
	class RWConstantBuffer;
	class StructuredBuffer;
	class RWStructuredBuffer;
	class Texture2D;
	class InstanceBuffer;
}

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
			ResourceState to, from;
		};

	public:

		virtual void SignalRecording() = 0;
		virtual void StartRecording() = 0;

		// transitions
		void ToRenderTarget(Ref<FrameBuffer> fb, ResourceState from) { Transition({ fb }, ResourceState::RenderTarget, from); }
		void ToSRV(Ref<FrameBuffer> fb, ResourceState from) { Transition({ fb }, ResourceState::ShaderResource, from); }
		void Present(ResourceState from) { Present(nullptr, from); }
		virtual void Present(Ref<FrameBuffer> fb, ResourceState from) = 0;

		virtual void Transition(std::vector<Ref<FrameBuffer>> fbs, ResourceState to, ResourceState from) = 0;
		virtual void Transition(std::vector<FBTransitionObject> transitions) = 0;

		virtual void Transition(std::vector<ResourceTransitionObject> transitions) = 0;

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
		virtual void SetConstantBuffer(uint32 index, Ref<RWConstantBuffer> buffer) = 0;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) = 0;
		virtual void SetStructuredBuffer(uint32 index, Ref<RWStructuredBuffer> buffer) = 0;
		virtual void SetRootConstant(uint32 index, uint32 data) = 0;
		void SetRootConstant(uint32 index, float data) { SetRootConstant(index, *(uint32*)&data); }
		virtual void SetTexture(uint32 index, Ref<Texture2D> texture) = 0;
		virtual void SetFrameBuffer(uint32 index, Ref<FrameBuffer> buffer, uint32 attatchment) = 0;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) = 0;
		virtual void ExecuteBundle(Ref<CommandList> commandList) = 0;

		// compute
		virtual void SetComputeShader(Ref<ComputeShader> shader) = 0;

		virtual void Close() = 0;

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);
	};
}
