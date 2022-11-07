#include "pch.h"
#include "SceneRenderer.h"
#include "Renderer.h"

#include "Platform/DirectX12/DirectX12ResourceManager.h"

namespace Engine
{

	// ObjectControlBlock
	void SceneRenderer::ObjectControlBlock::UpdateTransform(const Math::Mat4& transform)
	{
		InstanceData data = m_Object.m_Instances->Get<InstanceData>(m_InstanceLocation);
		data.m_Transform = transform;
		m_Object.m_Instances->SetData(m_InstanceLocation, 1, &data);
	}

	// RenderObject
	SceneRenderer::ObjectControlBlockRef SceneRenderer::RenderObject::AddInstance(const Math::Mat4& transform, Ref<Material> mat)
	{
		InstanceData data{ transform, mat->GetBuffer()->GetDescriptorLocation() };
		m_Instances->PushBack(1, &data); // create new instance
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
				m_Instances->SetData(controlBlock->m_InstanceLocation, 1, &m_Instances->Get<InstanceData>(block.m_InstanceLocation));
				//std::swap(m_Instances[controlBlock->m_InstanceLocation], m_Instances[block.m_InstanceLocation]);
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
	SceneRenderer::ObjectControlBlockRef SceneRenderer::ShaderDrawSection::AddObject(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform)
	{
		m_Objects.push_front({}); // create a new object
		RenderObject& object = m_Objects.front(); // get new object
		object.m_Mesh = mesh; // set mesh on object
		return object.AddInstance(transform, material); // add and return new object
	}


	// scene renderer
	SceneRenderer::SceneRenderer()
	{
		m_RenderGraph = CreateRef<RenderGraph>();
		m_RenderGraph->AddToCommandQueue();

		m_CameraIndexBuffer = ConstantBuffer::Create(sizeof(uint32));
	}

	void SceneRenderer::OnViewportResize(uint32 width, uint32 height)
	{
		m_RenderGraph->OnViewportResize(width, height);
		Invalidate();
	}

	void SceneRenderer::SetMainCamera(const Camera& camera)
	{
		uint32 bufferLoc = camera.GetCameraBuffer()->GetDescriptorLocation();
		m_CameraIndexBuffer->SetData(&bufferLoc);
	}

	void SceneRenderer::SetDirectionalLight(Ref<DirectionalLight> light)
	{
		SceneData& data = m_RenderGraph->GetScene();
		data.m_DirectinalLight = light;
		Invalidate();
	}

	void SceneRenderer::SetSkyBox(Ref<Texture> texture)
	{
		SceneData& data = m_RenderGraph->GetScene();
		data.m_Skybox = texture;
		Invalidate();
	}

	void SceneRenderer::UpdateBuffers()
	{
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			for (auto& renderObject : shaderDawSection)
			{
				//if (!renderObject.m_Instances->Empty())
					renderObject.m_Instances->Apply();
			}
		}
	}

	void SceneRenderer::Invalidate()
	{
		m_Invalid = true;
	}

	SceneRenderer::ObjectControlBlockRef SceneRenderer::Submit(Ref<Mesh> mesh, Ref<Material> material, const Math::Mat4& transform)
	{
		Invalidate();
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
		Invalidate();
		if(controlBlock)
			controlBlock->m_Object.RemoveInstance(controlBlock);
	}

	void SceneRenderer::Build()
	{
		if (m_Invalid)
		{
			CREATE_PROFILE_SCOPEI("Re Build command lists");
			m_Invalid = false;

			SceneData& data = m_RenderGraph->GetScene();
			data.m_MainCamera = m_CameraIndexBuffer;

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

			//m_Invalid = true;
			m_RenderGraph->Build();
		}

		m_RenderGraph->UpdateStates();
	}

	Ref<SceneRenderer> SceneRenderer::Create()
	{
		return CreateRef<SceneRenderer>();
	}
}

