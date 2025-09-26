#pragma once
#include "Engine/Core/Core.h"
#include "CPUCommands.h"
#include "Abstractions/Resources/Texture.h"
#include "Abstractions/Resources/FrameBuffer.h"
#include "Shaders/ShaderPass.h"

namespace tracy
{
	struct SourceLocationData;
}

namespace Engine
{

	enum CommandListType
	{
		Copy,
		Compute,
		Graphics
	};

	class CPUCommandAllocator
	{
	public:

		using ResourceStateMap = std::unordered_map<GPUResource*, ResourceState>;

	public:
		CPUCommandAllocator(const std::string& name) : m_Name(name) {}
		~CPUCommandAllocator();

		void PrependResourceStateCommands(const ResourceStateMap& resourceStates);
		void MergeResourceStatesInto(ResourceStateMap& resourceStates);
		void SetEndOfFrameStates();
		
		void SubmitCommand(CPUCommand* command) { m_Commands.Push(command); }
		CPUCommand* PeekCommand() { return m_Commands.Back(); }

		const Utils::Vector<CPUCommand*>& GetCommands() { return m_Commands; }

	private:
		Utils::Vector<CPUCommand*> m_Commands;

		Utils::Vector<ResourceStateObject> m_PendingTransitions;
		ResourceStateMap m_ResourceStates;
		std::string m_Name;

		friend class CPUCommandList;
	};

	class CPUCommandList
	{
	public:
		CPUCommandList() = default;
		~CPUCommandList();

		void SetName(const std::string& name) { m_Name = name; }
		const std::string& GetName() { return m_Name; }
		const Ref<ShaderPass> GetBoundShader() { return m_BoundShader; }

		CPUCommandAllocator* TakeAllocator();

		void StartRecording();
		void StopRecording();

		void BeginEvent(const char* eventName);
		void BeginEvent(const std::string& eventName);
		void EndEvent();

		void BeginGPUEvent(const tracy::SourceLocationData* data);
		void EndGPUEvent();

		// resource transitions
		void Present() { Present(m_RenderTarget); }
		void Present(Ref<FrameBuffer> fb);
		void ValidateStates(const Utils::Vector<ResourceStateObject>& resources);
		void ValidateState(ResourceStateObject resource) { ValidateStates({ resource }); }
		void ValidateState(GPUResource* resource, ResourceState state) { ValidateStates({ { resource, state } }); }
		void ValidateState(Ref<FrameBuffer> frameBuffer, ResourceState state = ResourceState::RenderTarget);

		// UAV
		void AwaitUAVs(Utils::Vector<GPUResource*> uavs);
		void AwaitUAV(GPUResource* uav) { AwaitUAVs({ uav }); }

		// transient
		void AllocateTransient(Ref<RenderTarget2D> renderTarget);
		void AllocateTransient(Ref<FrameBuffer> buffer);

		void CloseTransient(Ref<RenderTarget2D> renderTarget);
		void CloseTransient(Ref<FrameBuffer> buffer);

		// MSAA
		void ResolveMSAA(Ref<FrameBuffer> texture, Ref<FrameBuffer> msaaTexture);

		// copying
		void CopyBuffer(GPUResource* dest, uint64 destOffset, GPUResource* src, uint64 srcOffset, uint64 size);
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint64 size) { CopyBuffer(dest, 0, src, 0, size); }
		void CopyBuffer(GPUResource* dest, GPUResource* src) { CopyBuffer(dest, src, 0); }
		void CopyResource(GPUResource* dest, GPUResource* src) { CopyBuffer(dest, src); } // alias for CopyBuffer

		void UploadTexture(GPUResource* dest, UploadTextureResource* src);

		// set render targets and viewport
		void SetViewport(Math::Vector2 pos, Math::Vector2 size, Math::Vector2 depths = { 0,1 });
		void SetViewport(Ref<RenderTarget2D> target, Math::Vector2 depths = { 0,1 }) { SetViewport({ 0,0 }, { target->GetWidth(), target->GetHeight() }, depths); }
		void SetViewport(Ref<FrameBuffer> buffer, Math::Vector2 depths = { 0,1 }) { SetViewport(buffer->GetAttachment(0), depths); }
		void SetRenderTarget(Ref<RenderTarget2D> renderTarget) { SetRenderTarget(FrameBuffer::Create({ renderTarget })); }
		void SetRenderTarget(Ref<FrameBuffer> buffer);

		// clear render targets
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, const Math::Vector4& color);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment);
		void ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color);
		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget);
		void ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color);

		// set shaders
		void SetShader(Ref<GraphicsShaderPass> shader);
		void SetShader(Ref<ComputeShaderPass> shader);
		void SetShader(Ref<WorkGraphShaderPass> shader);

		// set data
		void SetRootConstant(uint32 index, uint32 data);
		template<typename T>
		void SetRootConstant(uint32 index, T data) { SetRootConstant(index, *(uint32*)&data); }
		template<>
		void SetRootConstant(uint32 index, bool data) { uint32 d = data; SetRootConstant(index, d); }
		void SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer);
		void SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer);
		void SetRWStructuredBuffer(uint32 index, Ref<RWStructuredBuffer> buffer);
		void SetTexture(uint32 index, Ref<Texture2D> texture);
		void SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav);
		void SetRWTexture(uint32 index, Ref<RWTexture2D> texture, uint32 mip = 0);

		template<typename T>
		void SetRootConstant(const std::string& name, T data) { SetRootConstant(m_BoundShader->GetUniformLocation(name), data); }
		void SetConstantBuffer(const std::string& name, Ref<ConstantBuffer> data) { SetConstantBuffer(m_BoundShader->GetUniformLocation(name), data); }
		void SetStructuredBuffer(const std::string& name, Ref<StructuredBuffer> data) { SetStructuredBuffer(m_BoundShader->GetUniformLocation(name), data); }
		void SetRWStructuredBuffer(const std::string& name, Ref<RWStructuredBuffer> data) { SetRWStructuredBuffer(m_BoundShader->GetUniformLocation(name), data); }
		void SetTexture(const std::string& name, Ref<Texture2D> data) { SetTexture(m_BoundShader->GetUniformLocation(name), data); }
		void SetRWTexture(const std::string& name, Texture2DUAVDescriptorHandle* uav) { SetRWTexture(m_BoundShader->GetUniformLocation(name), uav); }
		void SetRWTexture(const std::string& name, Ref<RWTexture2D> texture, uint32 mip = 0) { SetRWTexture(m_BoundShader->GetUniformLocation(name), texture, mip); }

		void SetVertexBuffer(uint32 stream, Ref<VertexBuffer> vb);
		void SetIndexBuffer(Ref<IndexBuffer> ib);

		// do work
		void DrawInstanced(uint32 numInstances = 1);
		void DrawMesh(Ref<Mesh> mesh);
		void DrawMesh(Ref<Mesh> mesh, Ref<VertexBuffer> instanceBuffer, uint32 numInstances = UINT32_MAX);
		void DrawMesh(Ref<Mesh> mesh, Utils::Vector<Ref<VertexBuffer>> instanceBuffers, uint32 numInstances = UINT32_MAX);
		void DrawMesh(Utils::Vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32 numInstances = 1);
		void DispatchGroups(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGroupsZ);
		void DispatchThreads(uint32 threadsX, uint32 threadsY, uint32 threadsZ);
		void DispatchGraph(void* data, uint32 stride, uint32 count);
		void DispatchGraph(uint32 numRecords = 1) { DispatchGraph(nullptr, 0, numRecords); }
		template<typename T>
		void DispatchGraph(const Utils::Vector<T>& data) { DispatchGraph((void*)data.Data(), data.ElementSize(), data.Count()); }
		void DispatchGraph(Ref<StructuredBuffer> buffer);

		static Ref<CPUCommandList> Create(CommandListType type = CommandListType::Graphics) { return CreateRef<CPUCommandList>(); }

	private:

		void FlushBindings();

		void Transition(const Utils::Vector<ResourceTransitionObject>& transitions);

	private:
		std::string m_Name = "";
		CPUCommandAllocator* m_CommandAllocator = nullptr;

		Ref<FrameBuffer> m_RenderTarget = nullptr;
		Ref<ShaderPass> m_BoundShader = nullptr;
		Utils::Vector<GPUDataBinding> m_Bindings;

		Utils::Vector<Ref<VertexBuffer>> m_BoundVertexBuffers;
		Ref<IndexBuffer> m_BoundIndexBuffer;

		Utils::Vector<std::string> m_EventStack;
	};
}
