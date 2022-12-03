#pragma once
#include <Engine/Math/Math.h>
#include "ConstantBuffer.h"
#include "Camera.h"
#include "FrameBuffer.h"
#include "StructuredBuffer.h"

namespace Engine
{

	class DirectionalLight
	{
	public:
		static constexpr uint32 s_NumShadowMaps = 5;
	private:
		//#pragma pack 4
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
				uint32 padding[2];
			};

			CascadedShadowMaps() = default;
			CascadedShadowMaps(Ref<Camera> camera);

			void UpdateMaps(Math::Vector3 dir);

		public:
			Ref<Camera> m_Camera;
			float m_BaseFactor = 5;

			Ref<FrameBuffer> m_ShadowMaps[s_NumShadowMaps];
			Ref<Camera> m_Cameras[s_NumShadowMaps];

			Ref<ConstantBuffer> m_CameraIndeces;
		};

	public:
		DirectionalLight(Math::Vector3 dir, Math::Vector3 color, float intensity) :
			m_Data(dir, color, intensity)
		{ m_Buffer = ConstantBuffer::Create(sizeof(DirectionalLightData)); }

		Math::Vector2 GetAngles() const { return m_Angles; }
		Math::Vector3 GetDirection() const { return m_Data.direction; }
		Math::Vector3 GetColor() const { return m_Data.color; }
		float GetIntensity() const { return m_Data.intensity; }
		float GetSize() const { return m_Data.size; }

		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 dir) { m_Data.direction = dir; }
		void SetColor(Math::Vector3 color) { m_Data.color = color; }
		void SetIntensity(float intensity) { m_Data.intensity = intensity; }
		void SetSize(float size) { m_Data.size = size; }

		void Apply() { m_Buffer->SetData(&m_Data); }

		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }

		void AddCamera(Ref<Camera> camera);

		void ClearCameras();
		void UpdateShadowMaps();

		std::unordered_map<Ref<Camera>, CascadedShadowMaps>& GetShadowMaps() { return m_ShadowMaps; }
		const std::unordered_map<Ref<Camera>, CascadedShadowMaps>& GetShadowMaps() const { return m_ShadowMaps; }

	private:
		DirectionalLightData m_Data;
		Ref<ConstantBuffer> m_Buffer;

		std::unordered_map<Ref<Camera>, CascadedShadowMaps> m_ShadowMaps;

		Math::Vector2 m_Angles = { 0,-90 };
	};

	struct PointLight
	{
		Math::Vector3 position = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 color = { 1.0f, 1.0f, 1.0f };
		float diffuseIntensity = 1;
		float attConst = 1;
		float attLin = 1;
		float attQuad = 1;
	};
}
