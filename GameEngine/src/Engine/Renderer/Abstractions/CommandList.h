#pragma once
#include "Engine/Core/Core.h"
#include "Resources/FrameBuffer.h"
#include "Resources/ResourceState.h"
#include "Engine/Core/Flag.h"
#include "Resources/UploadBuffer.h"

namespace Engine
{
	class Mesh;
	class Shader;
	class ShaderPass;
	class GraphicsShaderPass;
	class ComputeShaderPass;
	class WorkGraphShaderPass;
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
		CommandList(CommandListType type) :
			m_Type(type)
		{}

	protected:
		void SignalRecording() { m_RecordFlag.Signal(); }
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

	
		// copying
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint64 size) { CopyBuffer(dest, 0, src, 0, size); }
		virtual void CopyBuffer(GPUResource* dest, uint64 destOffset, GPUResource* src, uint64 srcOffset, uint64 size) = 0;

		virtual void CopyResource(GPUResource* dest, GPUResource* src) = 0;

		virtual void UploadTexture(GPUResource* dest, UploadTextureResource* src) = 0;


		// rendering
		virtual void SetRenderTarget(Ref<RenderTarget2D> renderTarget) = 0;
		virtual void SetRenderTarget(Ref<FrameBuffer> buffer) = 0;

		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color);

		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget);
		virtual void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color) = 0;

		virtual void SetShader(Ref<GraphicsShaderPass> shader) = 0;
		virtual void SetShader(Ref<ComputeShaderPass> shader) = 0;
		virtual void SetShader(Ref<WorkGraphShaderPass> shader) = 0;
		virtual void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer) = 0;
		virtual void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer) = 0;
		virtual void SetRWStructuredBuffer(uint32 index, Ref<RWStructuredBuffer> buffer) = 0;
		virtual void SetRootConstant(uint32 index, uint32 data) = 0;
		template<typename T>
		void SetRootConstant(uint32 index, T data) { SetRootConstant(index, *(uint32*)&data); }
		virtual void SetTexture(uint32 index, Ref<Texture2D> texture) = 0;
		void SetRWTexture(uint32 index, Ref<RWTexture2D> texture, uint32 mip = 0) { SetRWTexture(index, texture->GetUAVDescriptor(mip)); }
		virtual void SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav) = 0;
		virtual void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1) = 0;
		virtual void ExecuteBundle(Ref<CommandList> commandList) = 0;

		virtual void Dispatch(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGrouptsZ) = 0;

		virtual void DisbatchGraph(uint32 numRecords = 1) = 0;
		virtual void DisbatchGraph(Ref<StructuredBuffer> buffer) = 0;
		virtual void DisbatchGraph(void* data, uint32 stride, uint32 count) = 0;
		template<typename T>
		void DisbatchGraph(const std::vector<T>& data) { DisbatchGraph((void*)data.Data(), data.ElementSize(), data.Count()); }

		// mis
		virtual void AwaitUAV(GPUResource* uav) = 0;
		virtual void AwaitUAVs(std::vector<GPUResource*> uavs) = 0;

		virtual void Close() = 0;

		virtual std::vector<ResourceStateObject>& GetPendingTransitions() = 0;
		virtual std::unordered_map<GPUResource*, ResourceState> GetEndingResourceStates() = 0;

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);

		friend class CommandQueue;

	protected:
		Flag m_RecordFlag;

		Ref<FrameBuffer> m_RenderTarget;
		Ref<ShaderPass> m_BoundShader;

		CommandListType m_Type;

		RecordState m_State = CommandList::Closed;
	};
}
