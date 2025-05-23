#section config

passes = {
	DepthTestLinePass = {
		VS = vertex;
		PS = pixel;
		topology = line;
	};
	LinePass = {
		VS = vertex;
		PS = pixel;
		topology = line;
		depthTest = none;
	};
	DepthTestMeshPass = {
		VS = vertex;
		PS = pixel;
		topology = triangle;
	};
	MeshPass = {
		VS = vertex;
		PS = pixel;
		topology = triangle;
		depthTest = none;
	};

};




#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
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
	output.position = mul(ViewPorjection, input.position);
	output.color = input.color;
	return output;
}





#section pixel

Texture2D<float4> u_Textures[] : register(space0);
StaticSampler s_TextureSampler = StaticSampler(repeat, repeat, linear, liner);

PS_Output main(PS_Input input)
{
	PS_Output output;
	if (input.color.a == 0.0)
		discard;
	output.color = input.color;

	return output;
}
