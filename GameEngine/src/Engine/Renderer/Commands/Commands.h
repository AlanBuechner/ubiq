#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Resources/ResourceState.h"

namespace Engine
{
	class CommandList;
	class Renderer2D;
	class FrameBuffer;
	class ShaderPass;

	class ConstantBuffer;
	class StructuredBuffer;
	class Texture2D;
	class RenderTarget2D;
	class Texture2DUAVDescriptorHandle;
	class Mesh;
	class InstanceBuffer;
}

namespace Engine
{
	struct Command
	{
		static void InitCommands();

		void(*RecoardCommand)(CommandList& cmdList, Command& commandData);
	};

#define CommandBody(className) \
static void (*RecoardCommandFunc)(CommandList& cmd, Command& commandData);\
className() {RecoardCommand = RecoardCommandFunc;}

	struct TransitionCommand : public Command
	{
		CommandBody(TransitionCommand);
		std::vector<ResourceTransitionObject> m_Transitions;
	};

	struct SetRenderTargetCommand : public Command 
	{
		CommandBody(SetRenderTargetCommand);
		FrameBuffer* m_FrameBuffer;
	};

	struct ClearRenderTargetCommand : public Command
	{
		CommandBody(ClearRenderTargetCommand);
		RenderTarget2D* m_RenderTarget;
		Math::Vector4 m_Color;
	};

	struct SetShaderCommand : public Command 
	{
		CommandBody(SetShaderCommand);
		ShaderPass* m_ShaderPass;
		FrameBuffer* m_BoundFrameBuffer;
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
		ConstantBuffer* m_Buffer;
	};

	struct SetStructuredBufferCommand : public Command 
	{
		CommandBody(SetStructuredBufferCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		StructuredBuffer* m_Buffer;
	};

	struct SetTextureCommand : public Command
	{
		CommandBody(SetTextureCommand);
		bool m_IsComputeShader;
		uint32 m_Index;
		Texture2D* m_Texture;
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
		Mesh* m_Mesh;
		InstanceBuffer* m_Instances;
		uint32 m_NumberOfInstances;
	};

	struct DisbatchComputeCommand : public Command
	{
		CommandBody(DisbatchComputeCommand);
		uint32 m_ThreadGroupsX, m_ThreadGroupsY, m_ThreadGroupsZ;
	};

#undef CommandBody

}
