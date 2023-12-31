#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Resources/ResourceState.h"
#include "Engine/Renderer/Resources/Texture.h"

namespace Engine
{
	class CommandList;
	class Renderer2D;
	class FrameBuffer;
	class ShaderPass;

	class ConstantBufferCBVDescriptorHandle;
	class StructuredBufferSRVDescriptorHandle;
	class Texture2D;
	class RenderTarget2D;
	class Texture2DSRVDescriptorHandle;
	class Texture2DUAVDescriptorHandle;
	class Texture2DRTVDSVDescriptorHandle;
	class VertexBufferView;
	class IndexBufferView;
	class InstanceBufferView;

}

namespace Engine
{
	struct Command
	{
		static void InitCommands();

		void(*RecordCommand)(CommandList& cmdList, Command& commandData);
	};

#define CommandBody(className) \
static void (*RecordCommandFunc)(CommandList& cmd, Command& commandData);\
className() {RecordCommand = RecordCommandFunc;}

	struct TransitionCommand : public Command
	{
		CommandBody(TransitionCommand);
		std::vector<ResourceTransitionObject> m_Transitions;
	};

	struct SetRenderTargetCommand : public Command 
	{
		CommandBody(SetRenderTargetCommand);
		std::vector<Texture2DRTVDSVDescriptorHandle*> m_AttachmentHandles;
		bool m_HasDepthAttachment;
	};

	struct ClearRenderTargetCommand : public Command
	{
		CommandBody(ClearRenderTargetCommand);
		Texture2DRTVDSVDescriptorHandle* m_RenderTargetHandle;
		Math::Vector4 m_Color;
	};

	struct SetShaderCommand : public Command 
	{
		CommandBody(SetShaderCommand);
		ShaderPass* m_ShaderPass;
		std::vector<TextureFormat> m_FrameBufferSpecification;
	};

	struct SetRootConstantCommand : public Command
	{
		CommandBody(SetRootConstantCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		uint32 m_Data;
	};

	struct SetConstantBufferCommand : public Command
	{
		CommandBody(SetConstantBufferCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		ConstantBufferCBVDescriptorHandle* m_CBVHandle;
	};

	struct SetStructuredBufferCommand : public Command 
	{
		CommandBody(SetStructuredBufferCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		StructuredBufferSRVDescriptorHandle* m_SRVHandle;
	};

	struct SetTextureCommand : public Command
	{
		CommandBody(SetTextureCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		Texture2DSRVDescriptorHandle* m_SRVHandle;
	};

	struct SetRWTextureCommand : public Command
	{
		CommandBody(SetRWTextureCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		Texture2DUAVDescriptorHandle* m_UAVHandle;
	};

	struct DrawMeshCommand : public Command
	{
		CommandBody(DrawMeshCommand);
		VertexBufferView* m_VertexBufferView;
		IndexBufferView* m_IndexBufferView;
		InstanceBufferView* m_Instances;
		uint32 m_NumberOfInstances;
	};

	struct DisbatchComputeCommand : public Command
	{
		CommandBody(DisbatchComputeCommand);
		uint32 m_ThreadGroupsX, m_ThreadGroupsY, m_ThreadGroupsZ;
	};

#undef CommandBody

}
