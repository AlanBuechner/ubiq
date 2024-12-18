#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"

#include "ShaderPass.h"
#include "GraphicsShaderPass.h"
#include "ComputeShaderPass.h"
#include "WorkGraphShaderPass.h"

#include <string>
#include <Engine/Math/Math.h>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

namespace Engine
{
	struct ShaderParameter;
	struct ShaderSorce;
	struct MaterialParameter;
}

namespace Engine
{
	class Shader : public Asset
	{
	public:
		Shader(const std::string& src, const fs::path& file);

		Ref<GraphicsShaderPass> GetGraphicsPass(const std::string& passName);
		Ref<ComputeShaderPass> GetComputePass(const std::string& passName);
		Ref<WorkGraphShaderPass> GetWorkGraphPass(const std::string& passName);
		Utils::Vector<MaterialParameter>& GetParams();

		static Ref<Shader> Create(const fs::path& file);
		static Ref<Shader> CreateFromSrc(const std::string& src, const fs::path& file = "");

	private:
		std::unordered_map<std::string, Ref<GraphicsShaderPass>> m_GraphicsPasses;
		std::unordered_map<std::string, Ref<ComputeShaderPass>> m_ComputePasses;
		std::unordered_map<std::string, Ref<WorkGraphShaderPass>> m_WorkGraphPasses;
		Utils::Vector<MaterialParameter> m_Params;
	};
}
