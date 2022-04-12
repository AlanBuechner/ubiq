#pragma once
#include <Engine.h>
#include <string>
#include <Engine/Math/Math.h>


namespace Engine
{
	class PropertysPanel
	{
	public:
		static bool DrawFloatControl(const std::string& label, float& value, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawVec2Control(const std::string& label, Math::Vector2& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawVec3Control(const std::string& label, Math::Vector3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawTextureControl(const std::string& lable, Ref<Texture2D>& texture);


	};
}
