#pragma once
#include "Engine/Core/Core.h"
#include "Material.h"
#include "Mesh.h"
#include "SceneRenderer.h"
#include "Light.h"
#include "Engine/Core/Scene/Components.h"

namespace Engine
{
	struct CameraComponent : public Component
	{
		Engine::SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		virtual void OnTransformChange(const Math::Mat4& transform) override;
	};

	class DirectionalLightComponent : public Component
	{
		DirectionalLightComponent* s_Instance;
	public:
		virtual void OnComponentAdded() override;
		virtual void OnInvalid() override;
		virtual void OnComponentRemoved() override;
		
		void SetDirection(Math::Vector3 direction);
		void SetColor(Math::Vector3 color);
		void SetIntensity(float intensity);
		const DirectionalLight& GetDirectinalLight() { return m_Light; }

	private:
		DirectionalLight m_Light;
		Ref<ConstantBuffer> m_LightBuffer;

		bool m_Dirty = true;
	};

	class MeshRendererComponent : public Component
	{
	public:
		virtual void OnComponentRemoved() override;

		virtual void OnTransformChange(const Math::Mat4& transform);

		void SetMesh(Ref<Mesh> mesh) { m_Mesh = mesh; Invalidate(); }
		Ref<Mesh> GetMesh() { return m_Mesh; }

		void SetMaterial(Ref<Material> mat) { m_Mat = mat; Invalidate(); }
		Ref<Material> GetMaterial() { return m_Mat; }

		void Invalidate();

	private:
		Ref<Mesh> m_Mesh;
		Ref<Material> m_Mat;
		SceneRenderer::ObjectControlBlockRef m_Object = nullptr;
		Ref<SceneRenderer> m_SceneRenderer;
	};

	class SkyboxComponent : public Component
	{
		static SkyboxComponent* s_Instance;

	public:
		virtual void OnComponentAdded() override;
		virtual void OnComponentRemoved() override;

		Ref<Texture2D> GetSkyboxTexture() { return m_SkyboxTexture; }
		void SetSkyboxTexture(Ref<Texture2D> texture);

	private:
		Ref<Texture2D> m_SkyboxTexture;
	};
}
