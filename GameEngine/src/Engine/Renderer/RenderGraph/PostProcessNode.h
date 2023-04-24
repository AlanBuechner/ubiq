#pragma once
#include "RenderGraphNode.h"
#include "Engine/Renderer/PostProcessing/PostProcess.h"

namespace Engine
{
	class PostProcessNode : public RenderGraphNode
	{
	public:
		PostProcessNode(RenderGraph& graph);

		void SetRenderTarget(Ref<FrameBuffer> fb);
		void SetInput(const PostProcessInput& input) { m_Input = input; }
		void SetSrc(uint64 descriptorLocation) { m_SrcDescriptorLocation = descriptorLocation; }
		void AddPostProcess(Ref<PostProcess> post) { m_PostProcessStack.push_back(post); }

		void InitPostProcessStack();

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_RenderTarget;
		Ref<FrameBuffer> m_BackBuffer;
		uint64 m_SrcDescriptorLocation;

		std::vector<Ref<PostProcess>> m_PostProcessStack;

		Ref<Mesh> m_ScreenMesh;
		PostProcessInput m_Input;
	};
}
