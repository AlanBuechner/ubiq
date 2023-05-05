#pragma once

// reflection
#include <Reflection.h>

// general
#include "Engine/Core/Core.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Logging/Log.h"
#include "Engine/Core/Time.h"
#include "Engine/Util/Utils.h"

// Input
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/InputControler.h"
#include "Engine/Core/Input/InputControlerManeger.h"

// --- Renderer ---------------------------
// Renderers
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/LineRenderer.h"

#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Renderer/SwapChain.h"
#include "Engine/Renderer/GPUProfiler.h"

#include "Engine/Renderer/Components/SceneRendererComponents.h"

#include "Engine/Renderer/Mesh.h"
// Resources
#include "Engine/Renderer/Resources/Buffer.h"
#include "Engine/Renderer/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Resources/StructuredBuffer.h"
#include "Engine/Renderer/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Resources/Texture.h"
#include "Engine/Renderer/Resources/SubTexture.h"
#include "Engine/Renderer/Resources/FrameBuffer.h"
// Camera
#include "Engine/Renderer/Camera.h"
// ----------------------------------------

// ECS ------------------------------------
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/Entity.h"
#include "Engine/Core/Scene/Components.h"
// ----------------------------------------

// Physics --------------------------------
#include "Engine/Physics/Physics2D.h"
#include "Engine/Physics/PhysicsComponent.h"
// ----------------------------------------

// Mesh -----------------------------------
#include "Engine/Core/MeshBuilder.h"
#include "Engine/Core/MeshLoader.h"
// ----------------------------------------


// Math -----------------------------------
#include "Engine/Math/Math.h"
#include "Engine/Math/Types.h"
#include "Engine/Math/Vector.h"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Quaternion.h"
// ----------------------------------------

// Asset Manager --------------------------
#include "Engine/AssetManager/AssetManager.h"
// ----------------------------------------
