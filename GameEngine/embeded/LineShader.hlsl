#section config
topology = line;

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
	float4 color : COLOR;

	// semantics starting with "I_" are per instance data
	float4x4 transform : I_TRANSFORM;
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

cbuffer Camera
{
	float4x4 ViewPorjection;
};

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = mul(ViewPorjection, mul(input.transform, float4(input.position, 1)));
	output.color = input.color;
	return output;
}





#section pixel

PS_Output main(PS_Input input)
{
	PS_Output output;
	if (input.color.a == 0.0)
		discard;
	output.color = input.color;

	return output;
}
