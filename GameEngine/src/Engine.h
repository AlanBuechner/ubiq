#pragma once

#include "Engine/Core/core.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"
#include "Engine/Util/Utils.h"

// Input
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/InputControler.h"
#include "Engine/Core/Input/InputControlerManeger.h"

// --- Renderer ---------------------------
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"

#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/SubTexture.h"
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Renderer/VertexArray.h"
// Camera
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/PerspectiveCamera.h"
// ----------------------------------------

// ECS ------------------------------------
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/Components.h"
#include "Engine/Core/Scene/Entity.h"
#include "Engine/Core/Scene/ScriptableEntity.h"
// ----------------------------------------

// Physics --------------------------------
#include "Engine/Physics/Physics2D.h"
#include "Engine/Physics/PhysicsComponent.h"
// ----------------------------------------
