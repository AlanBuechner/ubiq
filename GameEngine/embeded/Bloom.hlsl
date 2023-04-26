#section config
topology = triangle;

passes = {
	downSample = {
		VS = vertex;
		PS = downSample;
	};
	upSample = {
		VS = vertex;
		PS = upSample;
		blendMode = add;
	};
	composit = {
		VS = vertex;
		PS = composit;
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

#section downSample

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

cbuffer RC_Threshold
{
	float threshold;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

float4 average(float4 a, float4 b, float4 c, float4 d)
{
	return (a + b + c + d) / 4.0;
}

float max3(float a, float b, float c)
{
	return max(a, max(b, c));
}

PS_Output main(PS_Input input)
{
	PS_Output output;
	Texture2D<float4> src = textures[srcLoc];

	uint2 srcTexelSize;
	src.GetDimensions(srcTexelSize.x, srcTexelSize.y);
	float2 srcPixelUVSize = 1.0 / srcTexelSize;


	/*
	
		00 .. 01 .. 02
		.. 09 .. 10 ..
	  ^ 03 .. 04 .. 05
	  | .. 11 .. 12 ..
	  y 06 .. 07 .. 08
		x->
	
	*/
	float4 samples[13] =
	{
		src.Sample(textureSampler, input.uv + (float2(-2, 2) * srcPixelUVSize)), // 0
		src.Sample(textureSampler, input.uv + (float2( 0, 2) * srcPixelUVSize)), // 1
		src.Sample(textureSampler, input.uv + (float2( 2, 2) * srcPixelUVSize)), // 2
		src.Sample(textureSampler, input.uv + (float2(-2, 0) * srcPixelUVSize)), // 3
		src.Sample(textureSampler, input.uv + (float2( 0, 0) * srcPixelUVSize)), // 4
		src.Sample(textureSampler, input.uv + (float2( 2, 0) * srcPixelUVSize)), // 5
		src.Sample(textureSampler, input.uv + (float2(-2,-2) * srcPixelUVSize)), // 6
		src.Sample(textureSampler, input.uv + (float2( 0,-2) * srcPixelUVSize)), // 7
		src.Sample(textureSampler, input.uv + (float2( 2,-2) * srcPixelUVSize)), // 8
		src.Sample(textureSampler, input.uv + (float2(-1, 1) * srcPixelUVSize)), // 9
		src.Sample(textureSampler, input.uv + (float2( 1, 1) * srcPixelUVSize)), // 10
		src.Sample(textureSampler, input.uv + (float2(-1,-1) * srcPixelUVSize)), // 11
		src.Sample(textureSampler, input.uv + (float2( 1,-1) * srcPixelUVSize)), // 12
	};

	float4 color = average(samples[9], samples[10], samples[11], samples[12]) * 0.5;
	color += average(samples[0], samples[1], samples[3], samples[4]) * 0.125;
	color += average(samples[1], samples[2], samples[4], samples[5]) * 0.125;
	color += average(samples[3], samples[4], samples[6], samples[7]) * 0.125;
	color += average(samples[4], samples[5], samples[7], samples[8]) * 0.125;

	//color = src.Sample(textureSampler, input.uv);
	if (threshold != 0.0)
	{
		const float k = 0.2;
		float br = max3(color.r, color.g, color.b);
		float rq = clamp(br - (threshold - k), 0.0, k*2);
		rq = (0.25/k) * rq * rq;
		color *= max(rq, br - threshold) / max(br, 0.000000001);
	}


	output.color = color;
	output.color.a = 1;
	return output;
}


#section upSample

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

PS_Output main(PS_Input input)
{
	PS_Output output;

	float4 srcColor = textures[srcLoc].Sample(textureSampler, input.uv);

	output.color = srcColor;
	output.color.a = 1;
	return output;
}


#section composit

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

cbuffer RC_Bloom
{
	uint bloomLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

PS_Output main(PS_Input input)
{
	PS_Output output;

	float4 srcColor = textures[srcLoc].Sample(textureSampler, input.uv);
	float4 bloom = textures[bloomLoc].Sample(textureSampler, input.uv);

	output.color = srcColor + bloom;
	output.color.a = 1;
	return output;
}
