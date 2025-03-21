#pragma once

// reflection
#include <Reflection.h>

// general
#include "Engine/Core/Core.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Logging/Log.h"
#include "Utils/Time.h"
#include "Utils/Utils.h"

// Input
#include "Engine/Core/Input/Input.h"

// --- Renderer ---------------------------
// Renderers
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/DebugRenderer.h"

#include "Engine/Renderer/Mesh.h"
// abstractions
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/CommandQueue.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
// Resources
#include "Engine/Renderer/Abstractions/Resources/SwapChain.h"
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/StructuredBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/Abstractions/Resources/SubTexture.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
// Camera
#include "Engine/Renderer/Camera.h"
// ----------------------------------------

// ECS ------------------------------------
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/Entity.h"
#include "Engine/Core/Scene/Components.h"
#include "Engine/Core/Scene/TransformComponent.h"
// ----------------------------------------

// Mesh -----------------------------------
#include "Engine/Core/MeshBuilder.h"
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
