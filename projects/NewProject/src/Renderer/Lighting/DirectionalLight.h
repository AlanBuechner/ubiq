#pragma once
#include "Engine/Core/Core.h"
#include "Light.h"

namespace Game
{
	class DirectionalLight
	{
	public:
		static constexpr uint32 s_NumShadowMaps = 5;
	private:
		// data that gets uploaded to the gpu
		struct DirectionalLightData
		{
			DirectionalLightData() = default;
			DirectionalLightData(Math::Vector3 dir, Math::Vector3 color, float intensity) :
				direction(dir), color(color), intensity(intensity)
			{}

			Math::Vector3 direction = { 0, -1, 0 };
			float padding = 69;
			Math::Vector3 color = { 1,1,1 };
			float intensity = 5;
			float size = 2;
		};

	public:
		struct CascadedShadowMaps
		{
			struct CascadeData
			{
				uint32 camera;
				uint32 frameBuffer;
				uint32 tWidth;
				uint32 tHeight;
				float minDist;
				float maxDist;
			};

			CascadedShadowMaps() = default;
			CascadedShadowMaps(Engine::Ref<Engine::Camera> camera);

			void UpdateMaps(Math::Vector3 dir);

		public:
			Engine::Ref<Engine::Camera> m_Camera;
			float m_BaseFactor = 5;

			Engine::Ref<Engine::FrameBuffer> m_ShadowMaps[s_NumShadowMaps];
			Engine::Ref<Engine::RWTexture2D> m_ShadowMapsTemp[s_NumShadowMaps];
			Engine::Ref<Engine::Camera> m_Cameras[s_NumShadowMaps];

			Engine::Ref<Engine::StructuredBuffer> m_CameraIndeces;
		};

	public:
		DirectionalLight(Math::Vector3 dir, Math::Vector3 color, float intensity);

		Math::Vector2 GetAngles() const { return m_Angles; }
		Math::Vector3 GetDirection() const { return m_Data.direction; }
		Math::Vector3 GetColor() const { return m_Data.color; }
		Math::Vector3 GetTint() const { return m_Tint; }
		float GetCCT() const { return m_CCT; }
		float GetIntensity() const { return m_Data.intensity; }
		float GetSize() const { return m_Data.size; }

		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 dir) { m_Data.direction = dir; m_Angles = Math::CartesianToSpherical(dir); }
		void SetTint(Math::Vector3 tint) { m_Tint = tint; m_Data.color = m_Tint * CCTToRGB(m_CCT); }
		void SetCCT(float cct) { m_CCT = cct; m_Data.color = m_Tint * CCTToRGB(m_CCT); }
		void SetIntensity(float intensity) { m_Data.intensity = intensity; }
		void SetSize(float size) { m_Data.size = size; }

		void Apply();

		Engine::Ref<Engine::ConstantBuffer> GetBuffer() { return m_Buffer; }

		void AddCamera(Engine::Ref<Engine::Camera> camera);
		Utils::Vector<Engine::Ref<Engine::Camera>> GetCameras();
		void RemoveCamera(Engine::Ref<Engine::Camera> camera);
		void ClearCameras();
		void UpdateShadowMaps();

		std::unordered_map<Engine::Ref<Engine::Camera>, CascadedShadowMaps>& GetShadowMaps() { return m_ShadowMaps; }
		const std::unordered_map<Engine::Ref<Engine::Camera>, CascadedShadowMaps>& GetShadowMaps() const { return m_ShadowMaps; }

	private:
		DirectionalLightData m_Data;
		Engine::Ref<Engine::ConstantBuffer> m_Buffer;

		std::unordered_map<Engine::Ref<Engine::Camera>, CascadedShadowMaps> m_ShadowMaps;

		Math::Vector2 m_Angles = { 0,-90 };

		float m_CCT = 6600;
		Math::Vector3 m_Tint;
	};
}
