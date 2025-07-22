#pragma once
#include "Abstractions/Resources/ResourceState.h"
#include "Abstractions/Resources/TextureFormat.h"
#include "Utils/Hash.h"

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
	class InstanceBuffer;
	class FrameBuffer;
	class RenderTarget2D;
	class UploadTextureResource;
	class Texture2DUAVDescriptorHandle;
	class ConstantBufferResource;
	class StructuredBufferSRVDescriptorHandle;
	class VertexBufferView;
	class IndexBufferView;
	class InstanceBufferView;
	class Texture2DRTVDSVDescriptorHandle;
	class StructuredBufferUAVDescriptorHandle;
	class Texture2DSRVDescriptorHandle;
	class StructuredBufferResource;
}

namespace Engine
{
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

	CREATE_COMMAND(ResourceTransitionCommand)
		Utils::Vector<ResourceTransitionObject> resourceStateTransitons;
	};

	CREATE_COMMAND(AwaitUAVCommand)
		Utils::Vector<GPUResource*> UAVs;
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
		Utils::Vector<TextureFormat> format;
	};

	CREATE_COMMAND(SetComputeShaderCommand)
		Ref<ComputeShaderPass> shaderPass;
	};

	CREATE_COMMAND(SetWorkGraphShaderCommand)
		Ref<WorkGraphShaderPass> shaderPass;
	};

	CREATE_COMMAND(SetRootConstantCommand)
		uint32 index;
		uint32 data;
		bool isCompute;
	};

	CREATE_COMMAND(SetConstantBufferCommand)
		uint32 index;
		ConstantBufferResource* res;
		bool isCompute;
	};

	CREATE_COMMAND(SetStructuredBufferCommand)
		uint32 index;
		StructuredBufferSRVDescriptorHandle* handle;
		bool isCompute;
	}; 
	
	CREATE_COMMAND(SetRWStructuredBufferCommand)
		uint32 index;
		StructuredBufferUAVDescriptorHandle* handle;
		bool isCompute;
	}; 
	
	CREATE_COMMAND(SetTextureCommand)
		uint32 index;
		Texture2DSRVDescriptorHandle* handle;
		bool isCompute;
	};

	CREATE_COMMAND(SetRWTextureCommand)
		uint32 index;
		Texture2DUAVDescriptorHandle* handle;
		bool isCompute;
	};

	CREATE_COMMAND(DrawMeshCommand)
		VertexBufferView* vertexBufferView;
		IndexBufferView* indexBufferView;
		InstanceBufferView* instanceBufferView;
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
