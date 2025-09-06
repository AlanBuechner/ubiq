#include "RenderPassObject.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Renderer/Lighting/DirectionalLight.h"
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"

namespace Game
{
	// ObjectControlBlock
	void ObjectControlBlock::UpdateTransform(const Math::Mat4& transform)
	{
		InstanceData& data = m_Object.m_InstanceData[m_DataIndex];
		data.m_Transform = transform;
	}

	void ObjectControlBlock::DestroySelf()
	{
		m_Object.RemoveInstance(this);
	}

	RenderObject::RenderObject()
	{
		m_InstanceData.Reserve(10); // default to capacity of 10
		m_Instances = Engine::VertexBuffer::Create(10, sizeof(InstanceData));
	}

	// RenderObject
	ObjectControlBlockRef RenderObject::AddInstance(const Math::Mat4& transform, Engine::Ref<Engine::Material> mat)
	{
		InstanceData data{ transform, mat->GetBuffer()->GetCBVDescriptor()->GetIndex() };
		m_InstanceData.Push(data); // create new instance
		m_ControlBlocks.push_front(ObjectControlBlock{ *this, (uint32)m_InstanceData.Count() - 1 }); // create new control block
		return &m_ControlBlocks.front(); // return control block
	}

	void RenderObject::RemoveInstance(ObjectControlBlockRef controlBlock)
	{
		if (controlBlock == nullptr)
			return;

		// remove data
		m_InstanceData.SwapAndPop(controlBlock->m_DataIndex);

		// swap data indexes for control blocks

		// find control block for last instance
		uint32 lastIndex = (uint32)m_Instances->GetCount() - 1; // last index in the instance buffer

		// get lats control block
		ObjectControlBlock* lastBlock = nullptr;
		for (auto& block : m_ControlBlocks)
		{
			if (block.m_DataIndex == lastIndex)
			{
				lastBlock = &block;
				break;
			}
		}
		CORE_ASSERT(lastBlock != nullptr, "some fucky shit happened", "");

		lastBlock->m_DataIndex = controlBlock->m_DataIndex; // swap the instance locations on the control blocks

		// remove the deleted control block
		m_ControlBlocks.remove_if([controlBlock](ObjectControlBlock& block) {return &block == controlBlock; });
	}

	void RenderObject::UpdateInstanceBuffer()
	{
		if (m_Instances->GetCount() < m_InstanceData.Capacity())
			m_Instances->Resize(m_InstanceData.Capacity());

		m_Instances->SetData(m_InstanceData);
	}

	// ShaderDrawSection
	ObjectControlBlockRef ShaderDrawSection::AddObject(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform)
	{
		m_Objects.push_front({}); // create a new object
		RenderObject& object = m_Objects.front(); // get new object
		object.m_Mesh = mesh; // set mesh on object
		return object.AddInstance(transform, material); // add and return new object
	}

	ObjectControlBlockRef RenderPassObject::Submit(Engine::Ref<Engine::Mesh> mesh, Engine::Ref<Engine::Material> material, const Math::Mat4& transform)
	{
		// find the correct shader
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			if (material->GetShader() == shaderDawSection.m_Shader)
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
		sec.m_Shader = material->GetShader();
		return sec.AddObject(mesh, material, transform);

	}

	void RenderPassObject::RemoveObject(ObjectControlBlockRef controlBlock)
	{
		if (controlBlock)
			controlBlock->m_Object.RemoveInstance(controlBlock);
	}

	void RenderPassObject::UpdateBuffers()
	{
		CREATE_PROFILE_FUNCTIONI();
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			for (auto& renderObject : shaderDawSection)
				renderObject.UpdateInstanceBuffer();
		}
	}

	void RenderPassObject::BuildDrawCommands(std::vector<DrawCommand>& outDrawCommands)
	{
		outDrawCommands.clear();
		for (auto& shaderDawSection : m_ShaderDrawSection)
		{
			outDrawCommands.reserve(shaderDawSection.m_Objects.size());
			for (auto& renderObject : shaderDawSection)
			{
				if (!renderObject.m_InstanceData.Empty())
				{
					DrawCommand cmd;
					cmd.m_Mesh = renderObject.m_Mesh;
					cmd.m_Shader = shaderDawSection.m_Shader;
					cmd.m_InstanceBuffer = renderObject.m_Instances;

					outDrawCommands.push_back(cmd);
				}
			}
		}
	}

}
