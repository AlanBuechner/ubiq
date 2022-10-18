#include "pch.h"
#include "SceneRenderer.h"
#include "Renderer.h"

#include "Platform/DirectX12/DirectX12ResourceManager.h"

namespace Engine
{

	// ObjectControlBlock
	void SceneRenderer::ObjectControlBlock::UpdateTransform(const Math::Mat4& transform)
	{
		m_Object.m_Instances[m_InstanceLocation].m_Transform = transform;
	}

	// RenderObject
	SceneRenderer::ObjectControlBlockRef SceneRenderer::RenderObject::AddInstance(const Math::Mat4& transform)
	{
		m_Instances.push_back({ transform, 0 }); // create new instance
		m_ControlBlocks.push_front(ObjectControlBlock{*this, (uint32)m_Instances.size()-1}); // create new control block
		return &m_ControlBlocks.front(); // return control block
	}

	void SceneRenderer::RenderObject::RemoveInstance(ObjectControlBlockRef controlBlock)
	{
		if (controlBlock == nullptr)
			return;

		// find control block for last instance
		uint32 swapIndex = (uint32)m_Instances.size() - 1; // last index in the instance buffer
		for (auto& block : m_ControlBlocks)
		{
			if (block.m_InstanceLocation == swapIndex)
			{
				// swap the buffer data
				std::swap(m_Instances[controlBlock->m_InstanceLocation], m_Instances[block.m_InstanceLocation]);
				m_Instances.pop_back(); // remove the last buffer

				block.m_InstanceLocation = controlBlock->m_InstanceLocation; // swap the instance locations on the control blocks

				// remove the deleted control block
				m_ControlBlocks.remove_if([controlBlock](ObjectControlBlock& block) {return &block == controlBlock; });

				return;
			}
		}

		CORE_ASSERT(false, "some fucky shit happened");
	}

	// ShaderDrawSection
	SceneRenderer::ObjectControlBlockRef SceneRenderer::ShaderDrawSection::AddObject(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform)
	{
		m_Objects.push_front({}); // create a new object
		RenderObject& object = m_Objects.front(); // get new object
		object.m_Mesh = mesh; // set mesh on object
		return object.AddInstance(transform); // add and return new object
	}


	// scene renderer
	SceneRenderer::SceneRenderer()
	{
		m_CameraBuffer = ConstantBuffer::Create(sizeof(Math::Mat4));
	}

	void SceneRenderer::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		m_ViewPorj = camera.GetProjectionMatrix() * Math::Inverse(transform);
		m_CameraBuffer->SetData(&m_ViewPorj);
	}

	void SceneRenderer::BeginScene(const EditorCamera& camera)
	{
		m_ViewPorj = camera.GetViewProjection();
		m_CameraBuffer->SetData(&m_ViewPorj);
	}

	void SceneRenderer::Invalidate()
	{
		m_DrawCommands.clear();
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			m_DrawCommands.reserve(shaderDawSection.m_Objects.size());
			for (auto& renderObject : shaderDawSection)
			{
				if (!renderObject.m_Instances.empty())
				{
					DrawCommand cmd;
					cmd.m_Mesh = renderObject.m_Mesh;
					cmd.m_Shader = shaderDawSection.m_Shader;
					cmd.m_InstanceBuffer = InstanceBuffer::Create(sizeof(InstanceData), (uint32)renderObject.m_Instances.size());
					cmd.m_InstanceBuffer->SetData(0, (uint32)renderObject.m_Instances.size(), renderObject.m_Instances.data());
					m_DrawCommands.push_back(cmd);
				}
			}
		}
	}

	SceneRenderer::ObjectControlBlockRef SceneRenderer::Submit(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform)
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
						return renderObject.AddInstance(transform);
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
		Ref<CommandList> commandList = Renderer::GetMainCommandList();
		for (DrawCommand& cmd : m_DrawCommands)
		{
			commandList->SetShader(cmd.m_Shader->GetPass("main"));
			commandList->SetConstantBuffer(0, m_CameraBuffer);
			commandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
		}
	}

	Ref<SceneRenderer> SceneRenderer::Create()
	{
		return CreateRef<SceneRenderer>();
	}
}

