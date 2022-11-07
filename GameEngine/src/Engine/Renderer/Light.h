#pragma once
#include <Engine/Math/Math.h>
#include "ConstantBuffer.h"

namespace Engine
{

	class DirectionalLight
	{

		struct DirectionalLightData
		{
			DirectionalLightData() = default;
			DirectionalLightData(Math::Vector3 dir, Math::Vector3 color, float intensity) :
				direction(dir), color(color), intensity(intensity)
			{}

			Math::Vector3 direction = { 0, -1, 0 };
			float padding1;
			Math::Vector3 color = { 1,1,1 };
			float padding2;
			float intensity = 1;
		};

	public:
		DirectionalLight(Math::Vector3 dir, Math::Vector3 color, float intensity) :
			m_Data(dir, color, intensity)
		{ m_Buffer = ConstantBuffer::Create(sizeof(DirectionalLightData)); }

		Math::Vector2 GetAngles() const { return m_Angles; }
		Math::Vector3 GetDirection() const { return m_Data.direction; }
		Math::Vector3 GetColor() const { return m_Data.color; }
		float GetIntensity() const { return m_Data.intensity; }

		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 dir) { m_Data.direction = dir; }
		void SetColor(Math::Vector3 color) { m_Data.color = color; }
		void SetIntensity(float intensity) { m_Data.intensity = intensity; }

		void Apply() { m_Buffer->SetData(&m_Data); }

		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }

	private:
		DirectionalLightData m_Data;
		Ref<ConstantBuffer> m_Buffer;

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
