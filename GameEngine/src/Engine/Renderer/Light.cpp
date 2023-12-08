#include "pch.h"
#include "Light.h"

#include "Resources/ConstantBuffer.h"
#include "Resources/StructuredBuffer.h"
#include "Camera.h"
#include "Resources/FrameBuffer.h"
#include "Resources/StructuredBuffer.h"

namespace Engine
{

	Math::Vector3 CCTToRGB(float k)
	{
		Math::Vector3 color;
		float temp = k / 100;

		if (temp <= 66)
			color.r = 255;
		else
		{
			color.r = temp - 60;
			color.r = 329.698727446 * Math::Pow(color.r, -0.1332047592);
			if (color.r < 0) color.r = 0;
			if (color.r > 255) color.r = 255;
		}
		color.r /= 255;

		if (temp <= 66)
		{
			color.g = temp;
			color.g = 99.4708025861 * Math::Log(color.g) - 161.1195681661;
		}
		else
		{
			color.g = temp - 60;
			color.g = 288.1221695283 * Math::Pow(color.g, -0.0755148492);
		}
		if (color.g < 0) color.g = 0;
		if (color.g > 255) color.g = 255;
		color.g /= 255;

		if (temp >= 66)
			color.b = 255;
		else
		{
			if (temp <= 19)
				color.b = 0;
			else
			{
				color.b = temp - 10;
				color.b = 138.5177312231 * log(color.b) - 305.0447927307;
			}
		}
		if (color.b < 0) color.b = 0;
		if (color.b > 255) color.b = 255;
		color.b /= 255;

		return color;
	}


	DirectionalLight::CascadedShadowMaps::CascadedShadowMaps(Ref<Camera> camera) :
		m_Camera(camera)
	{
		std::vector<CascadeData> cascadeData;

		float nearPlan = 0.1f;
		float size = m_BaseFactor;
		for (uint32 i = 0; i < s_NumShadowMaps; i++)
		{
			m_Cameras[i] = CreateRef<Camera>();
			uint32 width = 4096 - (i*512);
			uint32 height = 4096 - (i*512);
			//uint32 width = 1024;
			//uint32 height = 1024;
			m_ShadowMaps[i] = FrameBuffer::Create({
				RenderTarget2D::Create(width, height, 1, TextureFormat::R32_FLOAT, { 1, 1, 1, 1 }, true),
				RenderTarget2D::Create(width, height, 1, TextureFormat::Depth, {1,0,0,0}),
			});

			//m_ShadowMapsTemp[i] = RWTexture2D::Create(width, height, 1, TextureFormat::RGBA16_UNORM);

			cascadeData.push_back(
				CascadeData{
					m_Cameras[i]->GetCameraBuffer()->GetCBVDescriptor()->GetIndex(),
					m_ShadowMaps[i]->GetAttachment(0)->GetSRVDescriptor()->GetIndex(),
					width, height,
					nearPlan, nearPlan + size
				}
			);
			nearPlan += size;
			size *= 2;
		}

		m_CameraIndeces = StructuredBuffer::Create(sizeof(CascadeData), s_NumShadowMaps);
		m_CameraIndeces->SetData(cascadeData.data(), s_NumShadowMaps);
	}

	void DirectionalLight::CascadedShadowMaps::UpdateMaps(Math::Vector3 dir)
	{
		// because these are macros for some reason. thanks windows vary cool
#undef far
#undef near
		//dir = -dir;
		dir = Math::Normalize(dir);

		Math::Mat4 proj = m_Camera->GetProjectionMatrix();

		Math::Mat4 vp = Math::Inverse(m_Camera->GetCameraData().VPMatrix);

		Math::Vector3 viewDir = m_Camera->GetCameraData().ViewMatrix[2]; // get the camera forward direction
		// project the camera view vector onto the lights plain
		viewDir = viewDir - (Math::Dot(viewDir, dir) / Math::LengthSqr(dir)) * dir;
		viewDir = Math::Normalize(viewDir);
		Math::Vector3 rightDir = Math::Cross(viewDir, dir);
		Math::Mat4 TBN = Math::Mat3(rightDir, viewDir, dir);

		float nearPlan = 0.1f;
		float size = m_BaseFactor;

		for (uint32 i = 0; i < s_NumShadowMaps; i++)
		{
			Math::Vector4 nearPoint = proj * Math::Vector4(0, 0, nearPlan, 1);
			Math::Vector4 farPoint = proj * Math::Vector4(0, 0, (nearPlan + size), 1);
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
			//cd.Position = center;
			cd.ViewMatrix = Math::Inverse(TBN * Math::Translate(center));
			cd.ProjectionMatrix = Math::Ortho(minx, maxx, miny, maxy, (minz - 100) * 3, maxz);
			cd.InvProjection = Math::Inverse(cd.ProjectionMatrix);
			cd.VPMatrix = cd.ProjectionMatrix * cd.ViewMatrix;
			m_Cameras[i]->UpdateCameraBuffer();
		}
	}

	DirectionalLight::DirectionalLight(Math::Vector3 dir, Math::Vector3 color, float intensity) :
		m_Data(dir, color, intensity)
	{
		m_Angles.y = Math::Degrees(Math::Asin(dir.y));
		float c = Math::Acos(dir.x);
		float s = Math::Degrees(Math::Asin(dir.z));
		if (c > 0)
			m_Angles.x = s;
		else
			m_Angles.x = -s;
		
		m_Buffer = ConstantBuffer::Create(sizeof(DirectionalLightData));
	}

	void DirectionalLight::SetAngles(Math::Vector2 rot)
	{
		m_Angles = rot;
		rot = Math::Radians(rot);
		Math::Vector3 dir = Math::SphericalToCartesian(rot);
		/*Math::Vector3 dir = {
			Math::Cos(rot.x) * Math::Cos(rot.y),
			Math::Sin(rot.y),
			Math::Sin(rot.x) * Math::Cos(rot.y),
		};*/
		m_Data.direction = dir;
	}

	void DirectionalLight::Apply()
	{
		m_Buffer->SetData(&m_Data);
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

