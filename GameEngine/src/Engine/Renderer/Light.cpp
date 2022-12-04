#include "pch.h"
#include "Light.h"
#include "LineRenderer.h"

namespace Engine
{


	DirectionalLight::CascadedShadowMaps::CascadedShadowMaps(Ref<Camera> camera) :
		m_Camera(camera)
	{
		std::vector<CascadeData> cascadeData;

		float nearPlan = 0.1f;
		float size = m_BaseFactor;
		for (uint32 i = 0; i < s_NumShadowMaps; i++)
		{
			m_Cameras[i] = CreateRef<Camera>();

			FrameBufferSpecification fbSpec;
			fbSpec.Attachments = { { FrameBufferTextureFormat::Depth, { 1,0,0,0 } } };
			fbSpec.Width = 1000;
			fbSpec.Height = 1000;
			fbSpec.InitalState = FrameBufferState::SRV;
			m_ShadowMaps[i] = FrameBuffer::Create(fbSpec);

			cascadeData.push_back(
				CascadeData{
					m_Cameras[i]->GetCameraBuffer()->GetDescriptorLocation(),
					m_ShadowMaps[i]->GetAttachmentShaderDescriptoLocation(),
					fbSpec.Width, fbSpec.Height,
					nearPlan, nearPlan + size
				}
			);
			nearPlan += size;
			size *= 2;
		}

		m_CameraIndeces = ConstantBuffer::Create(sizeof(CascadeData) * s_NumShadowMaps);
		m_CameraIndeces->SetData(cascadeData.data());
	}

	void DirectionalLight::CascadedShadowMaps::UpdateMaps(Math::Vector3 dir)
	{
		// because these are macros for some reason. thanks windows vary cool
#undef far
#undef near
		dir = -dir;
		dir = Math::Normalize(dir);

		Math::Mat4 proj = m_Camera->GetProjectionMatrix();

		Math::Mat4 vp = Math::Inverse(m_Camera->GetCameraData().VPMatrix);

		Math::Vector3 viewDir = m_Camera->GetCameraData().ViewMatrix[2]; // get the camera forward direction
		viewDir.z = -viewDir.z;
		// project the camera view vector onto the lights plain
		viewDir = viewDir - (Math::Dot(viewDir, dir) / Math::LengthSqr(dir)) * dir;
		viewDir = Math::Normalize(viewDir);
		Math::Vector3 rightDir = Math::Cross(viewDir, dir);
		Math::Mat4 TBN = Math::Mat3(rightDir, viewDir, dir);

		float nearPlan = 0.1f;
		float size = m_BaseFactor;

		for (uint32 i = 0; i < s_NumShadowMaps; i++)
		{
			Math::Vector4 nearPoint = proj * Math::Vector4(0, 0, -nearPlan, 1);
			Math::Vector4 farPoint = proj * Math::Vector4(0, 0, -(nearPlan + size), 1);
			nearPlan += size;
			size *= 2;

			float near = nearPoint.z / nearPoint.w;
			float far = farPoint.z / farPoint.w;

			// get verts
			Math::Vector4 verts[] = {
				{ -1, 1, near, 1 },
				{ -1, 1, far, 1 },

				{ 1, 1, near, 1 },
				{ 1, 1, far, 1 },

				{ -1, -1, near, 1 },
				{ -1, -1, far, 1 },

				{ 1, -1, near, 1 },
				{ 1, -1, far, 1 },
			};

			// transform verts from NDC space to wold space
			for (uint32 j = 0; j < 8; j++)
			{
				verts[j] = vp * verts[j];
				verts[j] = verts[j] / verts[j].w;
			}

			// calculate the view matrix
			Math::Mat4 viewMatrix = Math::Inverse(TBN);

			// transform from world space to light space
			for (uint32 j = 0; j < 8; j++)
				verts[j] = viewMatrix * verts[j];

			// find the bounds
			float minx = FLT_MAX;
			float miny = FLT_MAX;
			float minz = FLT_MAX;

			float maxx = -FLT_MAX;
			float maxy = -FLT_MAX;
			float maxz = -FLT_MAX;

			for (uint32 j = 0; j < 8; j++)
			{
				if (verts[j].x < minx) minx = verts[j].x;
				if (verts[j].y < miny) miny = verts[j].y;
				if (verts[j].z < minz) minz = verts[j].z;

				if (verts[j].x > maxx) maxx = verts[j].x;
				if (verts[j].y > maxy) maxy = verts[j].y;
				if (verts[j].z > maxz) maxz = verts[j].z;
			}

			// find the center of the vertices in world space
			Math::Vector3 center = { 
				(minx + maxx),
				(miny + maxy),
				(minz + maxz) 
			};
			center /= 2;

			minx -= center.x;
			miny -= center.y;
			minz -= center.z;

			maxx -= center.x;
			maxy -= center.y;
			maxz -= center.z;

			// set the camera data
			Camera::CameraData& cd = m_Cameras[i]->m_CameraData;
			cd.Position = center;
			cd.ViewMatrix = Math::Inverse(TBN * Math::Translate(center));
			cd.ProjectionMatrix = Math::Ortho(minx, maxx, miny, maxy, (minz - 15) * 3, maxz);
			cd.InvProjection = Math::Inverse(cd.ProjectionMatrix);
			cd.VPMatrix = cd.ProjectionMatrix * cd.ViewMatrix;
			m_Cameras[i]->UpdateCameraBuffer();
		}
	}

	void DirectionalLight::SetAngles(Math::Vector2 rot)
	{
		m_Angles = rot;
		rot = { Math::Radians(rot.x), Math::Radians(rot.y) };
		Math::Vector3 dir = {
			Math::Cos(rot.x) * Math::Cos(rot.y),
			Math::Sin(rot.y),
			Math::Sin(rot.x) * Math::Cos(rot.y),
		};
		SetDirection(dir);
	}

	void DirectionalLight::AddCamera(Ref<Camera> camera)
	{
		m_ShadowMaps[camera] = CascadedShadowMaps(camera); // add new shadow map
	}

	void DirectionalLight::ClearCameras()
	{
		m_ShadowMaps.clear();
	}

	void DirectionalLight::UpdateShadowMaps()
	{
		for (auto& sm : m_ShadowMaps)
			sm.second.UpdateMaps(m_Data.direction);
	}

}

