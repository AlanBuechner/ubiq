#section config
topology = triangle;

passes = {
	downSample = {
		CS = downSample;
	};
	upSample = {
		CS = upSample;
	};
	composite = {
		CS = composite;
	};
};







#section common
#pragma enable_d3d12_debug_symbols

#define COMPUTE_SIZE 8











#section downSample

cbuffer RC_Threshold
{
	float threshold;
};

Texture2D<float4> src;
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

float4 average(float4 a, float4 b, float4 c, float4 d)
{
	return (a + b + c + d) / 4.0;
}

float max3(float a, float b, float c)
{
	return max(a, max(b, c));
}

RWTexture2D<float4> DstTexture;

[numthreads(COMPUTE_SIZE, COMPUTE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 destTexelCount;
	DstTexture.GetDimensions(destTexelCount.x, destTexelCount.y);
	float2 destTexelSize = 1.0/destTexelCount;
	float2 uv = ((float2)DTid.xy / (float2)destTexelCount) + (destTexelSize/2);

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
		src.SampleLevel(textureSampler, uv + (float2(-2, 2) * srcPixelUVSize), 0), // 0
		src.SampleLevel(textureSampler, uv + (float2( 0, 2) * srcPixelUVSize), 0), // 1
		src.SampleLevel(textureSampler, uv + (float2( 2, 2) * srcPixelUVSize), 0), // 2
		src.SampleLevel(textureSampler, uv + (float2(-2, 0) * srcPixelUVSize), 0), // 3
		src.SampleLevel(textureSampler, uv + (float2( 0, 0) * srcPixelUVSize), 0), // 4
		src.SampleLevel(textureSampler, uv + (float2( 2, 0) * srcPixelUVSize), 0), // 5
		src.SampleLevel(textureSampler, uv + (float2(-2,-2) * srcPixelUVSize), 0), // 6
		src.SampleLevel(textureSampler, uv + (float2( 0,-2) * srcPixelUVSize), 0), // 7
		src.SampleLevel(textureSampler, uv + (float2( 2,-2) * srcPixelUVSize), 0), // 8
		src.SampleLevel(textureSampler, uv + (float2(-1, 1) * srcPixelUVSize), 0), // 9
		src.SampleLevel(textureSampler, uv + (float2( 1, 1) * srcPixelUVSize), 0), // 10
		src.SampleLevel(textureSampler, uv + (float2(-1,-1) * srcPixelUVSize), 0), // 11
		src.SampleLevel(textureSampler, uv + (float2( 1,-1) * srcPixelUVSize), 0), // 12
	};

	float4 color = average(samples[9], samples[10], samples[11], samples[12]) * 0.5;
	color += average(samples[0], samples[1], samples[3], samples[4]) * 0.125;
	color += average(samples[1], samples[2], samples[4], samples[5]) * 0.125;
	color += average(samples[3], samples[4], samples[6], samples[7]) * 0.125;
	color += average(samples[4], samples[5], samples[7], samples[8]) * 0.125;

	//float4 color = src.SampleLevel(textureSampler, uv, 0);
	if (threshold != 0.0)
	{
		const float k = 0.2;
		float br = max3(color.r, color.g, color.b);
		float rq = clamp(br - (threshold - k), 0.0, k*2);
		rq = (0.25/k) * rq * rq;
		color *= max(rq, br - threshold) / max(br, 0.000000001);
	}


	DstTexture[DTid.xy] = color;
	
}










#section upSample

Texture2D<float4> src;
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

groupshared half3 gs_SrcColorCache[COMPUTE_SIZE * COMPUTE_SIZE];
groupshared half3 gs_DestColorCache[COMPUTE_SIZE * COMPUTE_SIZE];

RWTexture2D<half3> DstTexture;

[numthreads(COMPUTE_SIZE, COMPUTE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex )
{
	gs_DestColorCache[groupIndex] = DstTexture[DTid.xy];

	uint2 destTexelCount;
	DstTexture.GetDimensions(destTexelCount.x, destTexelCount.y);
	float2 destTexelSize = 1.0/destTexelCount;
	float2 uv = ((float2)DTid.xy / (float2)destTexelCount) + (destTexelSize/2);

	gs_SrcColorCache[groupIndex] = src.SampleLevel(textureSampler, uv, 0).rgb;

	GroupMemoryBarrierWithGroupSync();

	//float4 srcColor = src.SampleLevel(textureSampler, uv, 0);
	DstTexture[DTid.xy] = gs_DestColorCache[groupIndex] + gs_SrcColorCache[groupIndex];
}






#section composite



Texture2D<float4> src;
Texture2D<float4> bloomTex;
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

RWTexture2D<float4> DstTexture;

[numthreads(COMPUTE_SIZE, COMPUTE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 destTexelCount;
	DstTexture.GetDimensions(destTexelCount.x, destTexelCount.y);
	float2 destTexelSize = 1.0/destTexelCount;
	float2 uv = ((float2)DTid.xy / (float2)destTexelCount) + (destTexelSize/2);

	float4 srcColor = src.SampleLevel(textureSampler, uv, 0);
	float4 bloom = bloomTex.SampleLevel(textureSampler, uv, 0);

	DstTexture[DTid.xy] = srcColor + bloom;
}
