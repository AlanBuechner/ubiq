#include "pch.h"
#include "ImGuiLayer.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "imgui.h"
#include "Engine/Core/Application.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/Windows/Win.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/DirectX12/Resources/DirectX12SwapChain.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Platform/DirectX12/Resources/DirectX12Descriptors.h"
#include "Platform/DirectX12/Resources/DirectX12ResourceManager.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "Platform/DirectX12/DX.h"
#endif // PLATFORM_WINDOWS


#include <ImGuizmo.h>

namespace Engine
{

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		SetDarkThemeColors();

		// When viewport's are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		void* window = app.GetWindow().GetNativeWindow();

		// Setup Platform/Renderer bindings
#if defined(PLATFORM_WINDOWS)
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		// init windows
		ImGui_ImplWin32_Init(window);

		// init dx12
		wrl::ComPtr<ID3D12DescriptorHeap> heap = DirectX12ResourceManager::s_SRVHeap->GetHeap();
		Ref<DirectX12SwapChain> swapChain = std::dynamic_pointer_cast<DirectX12SwapChain>(Application::Get().GetWindow().GetSwapChain());
		CORE_ASSERT(ImGui_ImplDX12_Init(context->GetDevice().Get(), 
			swapChain->GetBufferCount(), DXGI_FORMAT_R8G8B8A8_UNORM,
			heap.Get(),
			heap->GetCPUDescriptorHandleForHeapStart(),
			heap->GetGPUDescriptorHandleForHeapStart()),
			"Faild to initalize imgui");
#endif

	}

	void ImGuiLayer::OnDetach()
	{
#if defined(PLATFORM_WINDOWS)
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
#endif
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		
		
	}

	void ImGuiLayer::OnEvent(Event* e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e->Handled |= e->IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e->Handled |= e->IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
#if defined(PLATFORM_WINDOWS)
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
#endif
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// start dockspace
		static bool dockSpaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		bool opt_fullscreen = opt_fullscreen_persistant;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)// ImGuiDockNodeFlags_PassthruDockspace)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Dock space
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
		}

	}

	void ImGuiLayer::End()
	{
		ImGui::End(); // end dockspace
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
	}

	void ImGuiLayer::Build()
	{
		CREATE_PROFILE_FUNCTIONI();
		ImGuiIO& io = ImGui::GetIO();

#ifdef PLATFORM_WINDOWS
		Ref<DirectX12SwapChain> swapChain = std::dynamic_pointer_cast<DirectX12SwapChain>(Application::Get().GetWindow().GetSwapChain());
		Ref<DirectX12CommandList> commandList = Renderer::GetMainCommandList<DirectX12CommandList>();
		Ref<RenderTarget2D> rt = swapChain->GetCurrentRenderTarget();
		GPUTimer::BeginEvent(commandList, "ImGui");
		//commandList->ValidateState({ rt->GetResource(), ResourceState::RenderTarget });
		commandList->SetRenderTarget(rt);
		commandList->ClearRenderTarget(rt, (Math::Vector4&)ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->GetCommandList());
		commandList->Present();
		GPUTimer::EndEvent(commandList);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)commandList->GetCommandList());
		}
#endif // PLATFORM_WINDOWS
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
}
