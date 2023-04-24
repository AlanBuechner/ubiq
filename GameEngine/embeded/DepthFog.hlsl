#section config
topology = triangle;

passes = {
	depthFog = {
		VS = vertex;
		PS = depthFog;
	};
};

#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float4 position : POSITION;
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

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 InvProjection;
	float4x4 ViewPorjection;

	float3 Position;
	float3 Rotation;
};

#section vertex

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = input.position;
	output.uv = float2(input.position.x+1, 1-input.position.y)/2;
	return output;
}

#section depthFog

struct TextureInput
{
	uint colorBuffer;
	uint depthBuffer;
};

ConstantBuffer<TextureInput> texInput;
ConstantBuffer<Camera> camera;

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(repeat, repeat, point, point);

PS_Output main(PS_Input input)
{
	PS_Output output;

	float depth = textures[texInput.depthBuffer].Sample(textureSampler, input.uv).r;
	float4 srcColor = textures[srcLoc].Sample(textureSampler, input.uv);
	output.color = lerp(srcColor, float4(0.2,0.2,0.2,1), depth);
	//output.color = srcColor;

	output.color.a = 1;
	return output;
}
