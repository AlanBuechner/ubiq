#pragma once
#include <Engine.h>
#include <string>
#include <Engine/Math/Math.h>


namespace Engine
{
	class PropertysPanel
	{
	public:
		static bool DrawBoolControl(const std::string& lable, bool& value, bool resetValue = false);
		static bool DrawFloatControl(const std::string& label, float& value, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawFloatSlider(const std::string& label, float& value, float min, float max, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawVec2Control(const std::string& label, Math::Vector2& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawVec3Control(const std::string& label, Math::Vector3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static bool DrawColorControl(const std::string& label, Math::Vector3& values);
		static bool DrawTextureControl(const std::string& label, Ref<Texture2D>& texture);
		static bool DrawMaterialControl(const std::string& label, Ref<Material>& mat);

		static bool DrawPropertyControl(void* object, uint64 typeID, const Reflect::Property* prop);


		typedef std::function<bool(void*, void*, uint64, const Reflect::Property*)> ExposePropertyFunc;
		static std::unordered_map<uint64, ExposePropertyFunc> s_ExposePropertyFunctions;
		class AddExposePropertyFunc
		{
		public:
			AddExposePropertyFunc(uint64 typeID, ExposePropertyFunc func) {
				s_ExposePropertyFunctions.emplace(typeID, func);
			}
		};
#define ADD_EXPOSE_PROP_FUNC(type) Engine::PropertysPanel::AddExposePropertyFunc CAT(Draw,__LINE__)(typeid(type).hash_code(), [](void* voidData, void* object, uint64 typeID, const Reflect::Property* prop)
	};
}
