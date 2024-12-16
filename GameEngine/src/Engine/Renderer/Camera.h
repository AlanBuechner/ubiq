#pragma once
#include "Abstractions/Resources/ConstantBuffer.h"
#include <Engine/Math/Math.h>

namespace Engine
{
	class Camera
	{

	public:
		struct CameraData
		{
			Math::Mat4 ViewMatrix = Math::Mat4(1.0f);
			Math::Mat4 ProjectionMatrix = Math::Mat4(1.0f);
			Math::Mat4 InvProjection = Math::Mat4(1.0f);
			Math::Mat4 VPMatrix = Math::Mat4(1.0f);

			Math::Vector3 Position;
			Math::Vector3 Rotation;
		};

	public:
		Camera() {
			m_CameraDataBuffer = ConstantBuffer::Create(sizeof(CameraData));
		}
		Camera(const Math::Mat4& projectionMatrix)
		{
			m_CameraData.ProjectionMatrix = projectionMatrix;
			m_CameraDataBuffer = ConstantBuffer::Create(sizeof(CameraData));
		}

		virtual ~Camera() = default;

		const Math::Mat4& GetViewMatrix() const { return m_CameraData.ViewMatrix; }
		const Math::Mat4& GetProjectionMatrix() const { return m_CameraData.ProjectionMatrix; }
		const Math::Mat4& GetViewProjectionMatrix() const { return m_CameraData.VPMatrix; }
		const Ref<ConstantBuffer> GetCameraBuffer() const { return m_CameraDataBuffer; }
		void UpdateCameraBuffer() { m_CameraDataBuffer->SetData(&m_CameraData); }
		const CameraData& GetCameraData() { return m_CameraData; }

	public:
		CameraData m_CameraData;
		Ref<ConstantBuffer> m_CameraDataBuffer;
		
	};
}
