#section config
topology = triangle;

passes = { 
	main = {
		VS = vertex;
		PS = pixel; 
	};
};

#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float3 position : POSITION;
	float2 uv : UV;

	// semantics starting with "I_" are per instance data
	float4x4 transform : I_TRANSFORM;
	float4 color : I_COLOR;
	uint textureID : I_TEXTUREID;
	float2 uvSize : I_UVSIZE;
	float2 uvPosition : I_UVPOSITION;
	int entityID : I_ENTITYID;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;
	uint textureID: TEXTUREID;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};

#section vertex

cbuffer u_Camera
{
	float4x4 ViewPorjection;
};

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = mul(ViewPorjection, mul(input.transform, float4(input.position, 1)));
	output.color = input.color;
	output.uv = (input.uv * input.uvSize) + input.uvPosition;
	output.textureID = input.textureID;
	return output;
}

#section pixel

Texture2D<float4> u_Textures[];
StaticSampler s_TextureSampler = StaticSampler(clamp, clamp, linear, linear);

PS_Output main(PS_Input input)
{
	PS_Output output;
	float4 color = u_Textures[input.textureID].Sample(s_TextureSampler, input.uv) * input.color;
	if (color.a == 0.0)
		discard;
	output.color = color;
	
	return output;
}
