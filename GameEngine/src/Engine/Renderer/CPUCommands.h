#pragma once
#include "Abstractions/Resources/ResourceState.h"
#include "Abstractions/Resources/TextureFormat.h"
#include "Abstractions/Resources/FrameBuffer.h"
#include "Utils/Hash.h"

namespace tracy
{
	struct SourceLocationData;
}

namespace Engine
{
	class Mesh;
	class Shader;
	class ShaderPass;
	class GraphicsShaderPass;
	class ComputeShaderPass;
	class WorkGraphShaderPass;
	class ConstantBuffer;
	class StructuredBuffer;
	class RWStructuredBuffer;
	class Texture2D;
	class RWTexture2D;
	class VertexBuffer;
	class IndexBuffer;
	class FrameBuffer;
	class RenderTarget2D;
	class UploadTextureResource;
	class Texture2DUAVDescriptorHandle;
	class ConstantBufferResource;
	class StructuredBufferSRVDescriptorHandle;
	class VertexBufferView;
	class IndexBufferView;
	class Texture2DRTVDSVDescriptorHandle;
	class StructuredBufferUAVDescriptorHandle;
	class Texture2DSRVDescriptorHandle;
	class StructuredBufferResource;
}

namespace Engine
{

	struct GPUDataBinding
	{
		enum class Type
		{
			RootConstant,
			ConstantBuffer,
			StructuredBuffer,
			RWStructuredBuffer,
			Texture2D,
			RWTexture2D,
		};

		uint32 index;
		Type type;
		bool isCompute;
		uint64 data; // could be a pointer to a resource or handle
	};


	struct CPUCommand
	{
		virtual ~CPUCommand();

		inline uint32 GetCommandID() { return m_CommandID; }

	protected:
		CPUCommand(uint32 commandID) : m_CommandID(commandID) {}

	protected:
		uint32 m_CommandID = 0;
	};

#define CREATE_COMMAND(name) \
	struct CPU##name : public CPUCommand { \
		virtual ~CPU##name() override = default;\
		static constexpr uint32 GetStaticCommandID() { return COMPILE_TIME_CRC32_STR(#name); }\
		CPU##name() : CPUCommand(GetStaticCommandID()) {}

	CREATE_COMMAND(BeginEventStaticCommand)
		const char* eventName;
	};

	CREATE_COMMAND(BeginEventDynamicCommand)
		std::string eventName;
	};

	CREATE_COMMAND(EndEventCommand)
	};

	CREATE_COMMAND(BeginGPUEventCommand)
		const tracy::SourceLocationData* data;
	};

	CREATE_COMMAND(EndGPUEventCommand)
	};

	CREATE_COMMAND(ResourceTransitionCommand)
		Utils::Vector<ResourceTransitionObject> resourceStateTransitons;
	};

	CREATE_COMMAND(AwaitUAVCommand)
		Utils::Vector<GPUResource*> UAVs;
	};

	CREATE_COMMAND(OpenTransientCommand)
		GPUResource* res;
		Utils::Vector<Descriptor*> descriptors;
	};

	CREATE_COMMAND(CloseTransientCommand)
		GPUResource* res;
	};
	
	CREATE_COMMAND(CopyBufferCommand)
		GPUResource* dest;
		uint64 destOffset;
		GPUResource* src;
		uint64 srcOffset;
		uint64 size; // if size is 0 the full buffer is to copied ignoring the offsets
	};

	CREATE_COMMAND(UploadTextureCommand)
		GPUResource* dest;
		UploadTextureResource* src;
	};

	CREATE_COMMAND(SetViewportCommand)
		Math::Vector2 pos;
		Math::Vector2 size;
		Math::Vector2 depths;
	};

	CREATE_COMMAND(SetRenderTargetCommand)
		Utils::Vector<Texture2DRTVDSVDescriptorHandle*> renderTargetHandles;
		Texture2DRTVDSVDescriptorHandle* depthStencil;
	};

	CREATE_COMMAND(ClearRenderTargetCommand)
		Texture2DRTVDSVDescriptorHandle* handle;
		Math::Vector4 color;
		bool isDepthStencil;
	};

	CREATE_COMMAND(SetGraphicsShaderCommand)
		Ref<GraphicsShaderPass> shaderPass;
		FrameBufferDescription fbDesc;
	};

	CREATE_COMMAND(SetComputeShaderCommand)
		Ref<ComputeShaderPass> shaderPass;
	};

	CREATE_COMMAND(SetWorkGraphShaderCommand)
		Ref<WorkGraphShaderPass> shaderPass;
	};

	CREATE_COMMAND(BindDataCommand)
		Utils::Vector<GPUDataBinding> bindings;
	};

	CREATE_COMMAND(DrawMeshCommand)
		Utils::Vector<VertexBufferView*> vertexBufferViews;
		IndexBufferView* indexBufferView;
		uint32 numIndices;
		uint32 numInstances;
	};

	CREATE_COMMAND(DispatchCommand)
		uint32 threadGroupsX;
		uint32 threadGroupsY;
		uint32 threadGroupsZ;
	};

	CREATE_COMMAND(DispatchGraphCPUDataCommand)
		Utils::Vector<byte> data; // owned by the command
		uint32 stride;
		uint32 count;
	};

	CREATE_COMMAND(DispatchGraphGPUDataCommand)
		StructuredBufferResource* res;
	};


#undef CREATE_COMMAND
}
