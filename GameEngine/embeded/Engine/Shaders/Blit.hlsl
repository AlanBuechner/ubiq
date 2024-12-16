#section config

passes = {
	BlitRaster = {
		VS = vertex;
		PS = BlitRaster;
	};
	Blit = {
		CS = Blit;
	};
	BlurX = {
		CS = BlurX;
	};
	BlurY = {
		CS = BlurY;
	};
};



#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float3 position : POSITION;
	float2 uv : UV;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};







#section vertex

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = float4(input.position, 1);
	output.uv = input.uv;
	return output;
}



#section BlitRaster

Texture2D<float4> src;
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

PS_Output main(PS_Input input)
{
	PS_Output output;
	output.color = src.Sample(textureSampler, input.uv);
	return output;
}



#section Blit

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

void SampleTexture(uint2 pixel, inout float4 color, uint2 texelSize)
{
	//if (pixel.x >= texelSize.x)
	//	pixel.x = texelSize.x - 1;
	//if (pixel.y >= texelSize.y)
	//	pixel.y = texelSize.y - 1;

	color += SrcTexture[pixel];
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 destTexelSize;
	DstTexture.GetDimensions(destTexelSize.x, destTexelSize.y);
	if (DTid.x < destTexelSize.x && DTid.y < destTexelSize.y)
	{
		uint2 texcoords = 2 * DTid.xy;

		float4 color = float4(0, 0, 0, 0);
		SampleTexture(texcoords + uint2(0, 0), color, destTexelSize);
		SampleTexture(texcoords + uint2(1, 0), color, destTexelSize);
		SampleTexture(texcoords + uint2(0, 1), color, destTexelSize);
		SampleTexture(texcoords + uint2(1, 1), color, destTexelSize);

		//Write the final color into the destination texture.
		DstTexture[DTid.xy] = color / 4.0;
	}
}




#section BlurX

#define computeSize 128

static const uint cashSize = computeSize+2+2;
groupshared float4 gs_cash[cashSize];

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

[numthreads(computeSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	
	if(GTid.x < 4)
		gs_cash[GTid.x] = SrcTexture[DTid.xy];
	gs_cash[GTid.x+4] = SrcTexture[DTid.xy + uint2(4,0)];
	
	GroupMemoryBarrierWithGroupSync();

	float4 color = float4(0, 0, 0, 0);
	for(uint i = 0; i < 5; i++)
		color += gs_cash[GTid.x + i];

	DstTexture[DTid.xy] = color / 5.0;
}


#section BlurY

#define computeSize 128

static const uint cashSize = computeSize+2+2;
groupshared float4 gs_cash[cashSize];

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

[numthreads(1, computeSize, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{

	if(GTid.x < 4)
		gs_cash[GTid.y] = SrcTexture[DTid.xy];
	gs_cash[GTid.y+4] = SrcTexture[DTid.xy + uint2(0,4)];
	
	GroupMemoryBarrierWithGroupSync();

	float4 color = float4(0, 0, 0, 0);
	for(uint i = 0; i < 5; i++)
		color += gs_cash[GTid.y + i];

	DstTexture[DTid.xy] = color / 5.0;

}
