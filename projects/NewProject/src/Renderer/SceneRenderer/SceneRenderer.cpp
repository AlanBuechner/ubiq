#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Abstractions/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Renderer/Lighting/DirectionalLight.h"
#include "Renderer/RenderGraph/RenderGraph.h"

namespace Game
{

	// ObjectControlBlock
	void SceneRenderer::ObjectControlBlock::UpdateTransform(const Math::Mat4& transform)
	{
		InstanceData& data = m_Object.m_Instances->Get<InstanceData>(m_InstanceLocation);
		data.m_Transform = transform;
	}

	SceneRenderer::RenderObject::RenderObject()
	{
		m_Instances = Engine::InstanceBuffer::Create(10, sizeof(InstanceData)); // default to capacity of 10
	}

	// RenderObject
	SceneRenderer::ObjectControlBlockRef SceneRenderer::RenderObject::AddInstance(const Math::Mat4& transform, Engine::Ref<Engine::Material> mat)
	{
		InstanceData data{ transform, mat->GetBuffer()->GetCBVDescriptor()->GetIndex() };
		m_Instances->PushBack(&data); // create new instance
		m_ControlBlocks.push_front(ObjectControlBlock{*this, (uint32)m_Instances->GetCount()-1}); // create new control block
		return &m_ControlBlocks.front(); // return control block
	}

	void SceneRenderer::RenderObject::RemoveInstance(ObjectControlBlockRef controlBlock)
	{
		if (controlBlock == nullptr)
			return;

		// find control block for last instance
		uint32 swapIndex = (uint32)m_Instances->GetCount() - 1; // last index in the instance buffer
		for (auto& block : m_ControlBlocks)
		{
			if (block.m_InstanceLocation == swapIndex)
			{
				// swap the buffer data
				m_Instances->Get<InstanceData>(controlBlock->m_InstanceLocation) = m_Instances->Get<InstanceData>(block.m_InstanceLocation);
				m_Instances->PopBack(); // remove the last buffer

				block.m_InstanceLocation = controlBlock->m_InstanceLocation; // swap the instance locations on the control blocks

				// remove the deleted control block
				m_ControlBlocks.remove_if([controlBlock](ObjectControlBlock& block) {return &block == controlBlock; });

				return;
			}
		}

		CORE_ASSERT(false, "some fucky shit happened");
	}

	// ShaderDrawSection
	SceneRenderer::ObjectControlBlockRef SceneRenderer::ShaderDrawSection::AddObject(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform)
	{
		m_Objects.push_front({}); // create a new object
		RenderObject& object = m_Objects.front(); // get new object
		object.m_Mesh = mesh; // set mesh on object
		return object.AddInstance(transform, material); // add and return new object
	}


	// scene renderer
	SceneRenderer::SceneRenderer()
	{
		m_RenderGraph = Engine::CreateRef<RenderGraph>();
	}

	void SceneRenderer::OnViewportResize(uint32 width, uint32 height)
	{
		m_RenderGraph->OnViewportResize(width, height);
	}

	void SceneRenderer::SetDirectionalLight(Engine::Ref<DirectionalLight> light)
	{
		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_DirectinalLight = light;
	}

	void SceneRenderer::SetSkyBox(Engine::Ref<Engine::Texture2D> texture)
	{
		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_Skybox = texture;
	}

	void SceneRenderer::UpdateBuffers()
	{
		CREATE_PROFILE_FUNCTIONI();
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			for (auto& renderObject : shaderDawSection)
			{
				//if (!renderObject.m_Instances->Empty())
					renderObject.m_Instances->Apply();
			}
		}
	}

	SceneRenderer::ObjectControlBlockRef SceneRenderer::Submit(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform)
	{
		// find the correct shader
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			if (material->shader == shaderDawSection.m_Shader)
			{
				// find the correct mesh for the shader
				for (auto& renderObject : shaderDawSection)
				{
					if (renderObject.m_Mesh == mesh) // check if 
						return renderObject.AddInstance(transform, material);
				}

				// cant find mesh
				return shaderDawSection.AddObject(mesh, material, transform); // create and return new object
			}
		}

		// cant find shader
		// add new shader
		m_ShaderDrawSection.push_back({});
		ShaderDrawSection& sec = m_ShaderDrawSection.back();
		sec.m_Shader = material->shader;
		return sec.AddObject(mesh, material, transform);
	}

	void SceneRenderer::RemoveObject(ObjectControlBlockRef controlBlock)
	{
		if(controlBlock)
			controlBlock->m_Object.RemoveInstance(controlBlock);
	}

	void SceneRenderer::Build()
	{

		CREATE_PROFILE_SCOPEI("Build command lists");

		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_MainCamera = m_MainCamera;

		// compile commands
		data.m_DrawCommands.clear();
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			data.m_DrawCommands.reserve(shaderDawSection.m_Objects.size());
			for (auto& renderObject : shaderDawSection)
			{
				if (!renderObject.m_Instances->Empty())
				{
					DrawCommand cmd;
					cmd.m_Mesh = renderObject.m_Mesh;
					cmd.m_Shader = shaderDawSection.m_Shader;
					cmd.m_InstanceBuffer = renderObject.m_Instances;

					data.m_DrawCommands.push_back(cmd);
				}
			}
		}

		m_RenderGraph->Build();
	}

	void SceneRenderer::Render(Engine::Ref<Engine::CommandQueue> queue)
	{
		m_RenderGraph->Submit(queue);
	}

	Engine::Ref<SceneRenderer> SceneRenderer::Create()
	{
		return Engine::CreateRef<SceneRenderer>();
	}
}
