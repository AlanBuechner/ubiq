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

		enum RecordState
		{
			Open,
			RequestClose,
			Closed
		};

	protected:
		virtual void SignalRecording() = 0;
		virtual void InternalClose() = 0;

		virtual void Transition(std::vector<ResourceTransitionObject> transitions) = 0;

	public:

		virtual void StartRecording() = 0;

		// transitions
		void Present() { Present(nullptr); }
		virtual void Present(Ref<FrameBuffer> fb) = 0;


		virtual void ValidateStates(std::vector<ResourceStateObject> resources) = 0;
		void ValidateState(ResourceStateObject resource) { ValidateStates({ resource }); }
		void ValidateState(GPUResource* resource, ResourceState state) { ValidateStates({ { resource, state } }); }


		// rendering
		virtual void SetRenderTarget(Ref<RenderTarget2D> renderTarget) = 0;
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) = 0;

		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color);

		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget);
		virtual void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color) = 0;

		virtual void SetShader(Ref<ShaderPass> shader) = 0;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) = 0;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) = 0;
		virtual void SetRootConstant(uint32 index, uint32 data) = 0;
		void SetRootConstant(uint32 index, float data) { SetRootConstant(index, *(uint32*)&data); }
		virtual void SetTexture(uint32 index, Ref<Texture2D> texture) = 0;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) = 0;
		virtual void ExecuteBundle(Ref<CommandList> commandList) = 0;

		// compute
		virtual void SetComputeShader(Ref<ComputeShader> shader) = 0;

		// mis
		virtual void Close() = 0;

		virtual std::vector<ResourceStateObject>& GetPendingTransitions() = 0;
		virtual std::unordered_map<GPUResource*, ResourceState> GetEndingResourceStates() = 0;

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);

		friend class CommandQueue;
	};
}
