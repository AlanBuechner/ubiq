#include "pch.h"
#include "Input.h"

namespace Engine
{
	KeyCode* KeyCode::s_Instance = new KeyCode();

	KeyCode::KeyCode()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = this;
		}
	}
}