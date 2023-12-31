#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Resources/FrameBuffer.h"
#include "Engine/Renderer/Resources/ResourceState.h"
#include "Engine/Core/Flag.h"

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
	class Command;
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

		void Transition(std::vector<ResourceTransitionObject> transitions);

	public:
		void SignalRecording() { m_RecordFlag.Signal(); }

		virtual void Build(std::unordered_map<GPUResource*, ResourceState>& startingStates) = 0;

		void StartRecording();
		void Close();

		// transitions
		void Present() { Present(m_RenderTarget); }
		void Present(Ref<FrameBuffer> fb);


		void ValidateState(ResourceStateObject resource) { ValidateStates({ resource }); }
		void ValidateState(GPUResource* resource, ResourceState state) { ValidateStates({ { resource, state } }); }
		void ValidateStates(std::vector<ResourceStateObject> resources);


		// set render target
		void SetRenderTarget(Ref<RenderTarget2D> renderTarget) { SetRenderTarget(FrameBuffer::Create({ renderTarget })); }
		void SetRenderTarget(Ref<FrameBuffer> buffer);

		// clear render targets
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color);
		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget);
		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color);

		// shaders
		void SetShader(Ref<ShaderPass> shader);
		
		// root constant
		template<typename T>
		void SetRootConstant(uint32 index, T data) { SetRootConstant(index, *(uint32*)&data); }
		void SetRootConstant(uint32 index, uint32 data);

		// buffers
		void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer);
		void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer);

		// textures
		void SetTexture(uint32 index, Ref<Texture2D> texture);
		void SetRWTexture(uint32 index, Ref<RWTexture2D> texture, uint32 mip) { SetRWTexture(index, texture->GetUAVDescriptor(mip)); }
		void SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav);

		// execution
		void DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer = nullptr, int numInstances = -1);
		void DispatchCompute(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGroupsZ);

		std::vector<ResourceStateObject>& GetPendingTransitions() { return m_PendingTransitions; }
		std::unordered_map<GPUResource*, ResourceState> GetEndingResourceStates() { return m_ResourceStates; }

		static Ref<CommandList> Create(CommandListType type = CommandListType::Direct);

		friend class CommandQueue;

	protected:
		Flag m_RecordFlag;

		Ref<FrameBuffer> m_RenderTarget;
		Ref<ShaderPass> m_BoundShader;

		CommandListType m_Type;

		std::vector<Command*> m_Commands;

		std::vector<ResourceStateObject> m_PendingTransitions;
		std::unordered_map<GPUResource*, ResourceState> m_ResourceStates;

		RecordState m_State = CommandList::Closed;
	};
}
