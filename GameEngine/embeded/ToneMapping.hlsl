#section config
topology = triangle;

passes = {
	HillACES = {
		VS = vertex;
		PS = HillACES;
	};
	NarkowiczACES = {
		VS = vertex;
		PS = NarkowiczACES;
	};
	Uncharted = {
		VS = vertex;
		PS = Uncharted;
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

#section HillACES

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(repeat, repeat, point, point);

static const float3x3 ACESInputMat =
{
	{0.59719, 0.35458, 0.04823},
	{0.07600, 0.90834, 0.01566},
	{0.02840, 0.13383, 0.83777}
};

static const float3x3 ACESOutputMat =
{
	{ 1.60475, -0.53108, -0.07367},
	{-0.10208,  1.10813, -0.00605},
	{-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
	float3 a = v * (v + 0.0245786f) - 0.000090537f;
	float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> src = textures[srcLoc];

	float3 color = src.Sample(textureSampler, input.uv).rgb;

	color = mul(ACESInputMat, color);
	color = RRTAndODTFit(color);
	color = mul(ACESOutputMat, color);

	color = saturate(color);

	color = saturate(pow(abs(color), 1.0 / 2.2));

	output.color.rgb = color;
	output.color.a = 1;
	return output;
}


#section NarkowiczACES

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(repeat, repeat, point, point);


PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> src = textures[srcLoc];

	float3 color = src.Sample(textureSampler, input.uv).rgb;

	color = saturate((color * (2.51f * color + 0.03f)) / (color * (2.43f * color + 0.59f) + 0.14f));
	color = saturate(pow(abs(color), 1.0/2.2));

	output.color.rgb = color;
	output.color.a = 1;
	return output;
}

#section Uncharted

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(repeat, repeat, point, point);

float3 uncharted2_tonemap_partial(float3 x)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 uncharted2_filmic(float3 v)
{
	float exposure_bias = 2.0f;
	float3 curr = uncharted2_tonemap_partial(v * exposure_bias);

	float3 W = float3(11.2f, 11.2f, 11.2f);
	float3 white_scale = float3(1.0f, 1.0f, 1.0f) / uncharted2_tonemap_partial(W);
	return curr * white_scale;
}

PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> src = textures[srcLoc];

	float3 color = src.Sample(textureSampler, input.uv).rgb;

	color = uncharted2_filmic(color);

	output.color.rgb = color;
	output.color.a = 1;
	return output;
}
