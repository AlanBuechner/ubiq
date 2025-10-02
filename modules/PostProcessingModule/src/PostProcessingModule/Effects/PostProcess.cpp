#include "PostProcess.h"

namespace Game
{

	void PostProcess::Initialize(Engine::Ref<Engine::CPUCommandList> commandList, PostProcessInput* input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		m_Input = input;
		m_ScreenMesh = screenMesh;
		m_CommandList = commandList;
		Init();
	}

}

