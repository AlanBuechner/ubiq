#ifndef SHADOWS_HLSLI
#define SHADOWS_HLSLI

#include "Common.hlsli"
#include "Hash.hlsli"
#include "LightingCommon.hlsli"
#include "Engine/Shaders/Include/Camera.hlsli"

#define DEPTH_BIAS (0.00001)



StaticSampler s_ShadowSampler = StaticSampler(clamp, clamp, point, point);





// ---------------------- Hard Shadows ---------------------- //
float HardShadow(Texture2D shadowMap, float4 coords, float depthBias = DEPTH_BIAS)
{
	float depthSample = shadowMap.Sample(s_ShadowSampler, coords.xy).r + depthBias;
	return (depthSample < coords.z ? 0 : 1);
}








// ---------------------- Moment Shadows ---------------------- //
float4 CompressMoments(float4 moments)
{
	float4x4 mat = {
		1.5,			0,		-2,				 0,
		0,				4,		 0,				-4,
		0.86602540378,	0,		-0.38490017946,	 0,
		0,				0.5,	 0,				 0.5	
	};

	return  mul(mat, moments) + float4(0.5, 0, 0.5, 0);
}

float4 DecompressMoments(float4 moments)
{
	float4x4 mat = {
		-1/3,	 0,		1.73205080757,	0,
		 0,		 0.125,	0,				1,
		-0.75,	 0,		1.29903810568,	0,
		 0,		-0.125,	0,				1	
	};

	return  mul(mat, moments - float4(0.5, 0, 0.5, 0));
}

float4 BiasMoments(float4 moments, float a)
{
	return (1.0-a) * moments + a * float4(0, 0.63, 0, 0.63);
}


float MomentShadow(Texture2D shadowMap, float4 coords, float depthBias = DEPTH_BIAS)
{
	float depth = (coords.z * 2) - 1; // remap to [-1,1]
	float4 moments = shadowMap.Sample(s_ShadowSampler, coords.xy);
	//moments = DecompressMoments(moments);
	moments = BiasMoments(moments, 6 * pow(10, -5));
	float4 b = moments;
	
	float3 z;
	z[0] = depth - depthBias;

	float L21D11 = mad(-b[0], b[1], b[2]);
	float D11 = mad(-b[0], b[0], b[1]);
	float SquaredDepthVariance = mad(-b[1], b[1], b[3]);
	float D22D11 = dot(float2(SquaredDepthVariance, -L21D11), float2(D11, L21D11));
	float InvD11 = 1.0f / D11;
	float L21 = L21D11 * InvD11;
	float D22 = D22D11 * InvD11;
	float InvD22 = 1.0f / D22;

	float3 c = float3(1.0f, z[0], z[0] * z[0]);
	c[1] -= b.x;
	c[2] -= b.y + L21 * c[1];
	c[1] *= InvD11;
	c[2] *= InvD22;
	c[1] -= L21 * c[2];
	c[0] -= dot(c.yz, b.xy);
	float InvC2 = 1.0f / c[2];
	float p = c[1] * InvC2;
	float q = c[0] * InvC2;
	float D = (p * p * 0.25f) - q;
	float r = sqrt(D);
	z[1] = -p * 0.5f - r;
	z[2] = -p * 0.5f + r;
	float4 Switch =
		(z[2] < z[0]) ? float4(z[1], z[0], 1.0f, 1.0f) : (
		(z[1] < z[0]) ? float4(z[0], z[1], 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f));
	float Quotient = (Switch[0] * z[2] - b[0] * (Switch[0] + z[2]) + b[1]) / ((z[2] - Switch[1]) * (z[0] - z[1]));
	float shadowIntensity = Switch[2] + Switch[3] * Quotient;
	return saturate(shadowIntensity);
}









// ---------------------- PCSS Shadows ---------------------- //
#define DITHER_OFFSETS 32
#define BLOCKER_SEARCH_NUM_SAMPLES 32
#define PCF_NUM_SAMPLES 32

static float2 s_PoissonDisk[PCF_NUM_SAMPLES] = {
	float2(0.000000, 0.000000),
	float2(-0.132461, -0.121293),
	float2(0.022314, 0.253018),
	float2(0.189118, -0.246999),
	float2(-0.353666, 0.062870),
	float2(0.339090, 0.215194),
	float2(-0.114755, -0.424711),
	float2(-0.218390, 0.422034),
	float2(0.476877, -0.175078),
	float2(-0.498447, -0.204630),
	float2(0.241825, 0.513908),
	float2(0.176943, -0.568797),
	float2(-0.537507, 0.313341),
	float2(0.632855, 0.137292),
	float2(-0.388137, -0.548600),
	float2(-0.087181, 0.690123),
	float2(0.547756, -0.464858),
	float2(-0.740005, -0.027916),
	float2(0.542193, 0.535418),
	float2(-0.039334, -0.781893),
	float2(-0.511995, 0.618888),
	float2(0.815204, -0.113412),
	float2(-0.693763, -0.477881),
	float2(0.193172, 0.839417),
	float2(0.433539, -0.765662),
	float2(-0.854074, 0.277505),
	float2(0.833483, 0.379494),
	float2(-0.365243, -0.858816),
	float2(-0.316319, 0.896196),
	float2(0.853351, -0.455276),
	float2(-0.952834, -0.244641),
	float2(0.546395, 0.837528),
};

static float2 s_DitherOffsets[DITHER_OFFSETS] = {
	float2(0.680375, -0.211234),
	float2(0.566198, 0.596880),
	float2(0.823295, -0.604897),
	float2(-0.329554, 0.536459),
	float2(-0.444451, 0.107940),
	float2(-0.045206, 0.257742),
	float2(-0.270431, 0.026802),
	float2(0.904459, 0.832390),
	float2(0.271423, 0.434594),
	float2(-0.716795, 0.213938),
	float2(-0.967399, -0.514226),
	float2(-0.725537, 0.608353),
	float2(-0.686642, -0.198111),
	float2(-0.740419, -0.782382),
	float2(0.997849, -0.563486),
	float2(0.025865, 0.678224),
	float2(0.225280, -0.407937),
	float2(0.275105, 0.048574),
	float2(-0.012834, 0.945550),
	float2(-0.414966, 0.542715),
	float2(0.053490, 0.539828),
	float2(-0.199543, 0.783059),
	float2(-0.433371, -0.295083),
	float2(0.615449, 0.838053),
	float2(-0.860489, 0.898654),
	float2(0.051991, -0.827888),
	float2(-0.615572, 0.326454),
	float2(0.780465, -0.302214),
	float2(-0.871657, -0.959954),
	float2(-0.084597, -0.873808),
	float2(-0.523440, 0.941268),
	float2(0.804416, 0.701840),
};


float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
{
	return (zReceiver - zBlocker) / zBlocker;
}

void FindBlocker(Texture2D shadowMap, out float avgBlockerDepth, out float numBlockers, float2 uv, float zReceiver, float2 searchSize, float depthBias = DEPTH_BIAS)
{
	//This uses similar triangles to compute what
	float blockerSum = 0;
	numBlockers = 0;

	for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
	{
		float2 offset = s_PoissonDisk[i] * searchSize;
		float shadowMapDepth = shadowMap.SampleLevel(s_ShadowSampler, uv + offset, 0).r + depthBias;
		if (shadowMapDepth < zReceiver) {
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
	avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter(Texture2D shadowMap, float2 uv, float zReceiver, float2 filterRadiusUV, int ditherIndex, float depthBias = DEPTH_BIAS)
{
	float sum = 0.0f;
	for (int i = 0; i < PCF_NUM_SAMPLES; ++i)
	{
		float2 ditherOffset = s_DitherOffsets[(ditherIndex + i) % DITHER_OFFSETS] * (3.141592654 / PCF_NUM_SAMPLES); // add dithering
		float2 offset = (s_PoissonDisk[i] + ditherOffset) * filterRadiusUV;
		float depthSample = shadowMap.Sample(s_ShadowSampler, uv + offset).r + depthBias;
		sum += (depthSample < zReceiver ? 0 : 1);
	}
	return sum / PCF_NUM_SAMPLES;
}

float PCSSDirectional(Texture2D shadowMap, float4 coords, float4x4 ortho, float lightSize, float3 pixelLocation, float depthBias = DEPTH_BIAS)
{
	float4 p = mul(ortho, float4(1, 1, 1, 1));
	float2 orthoSize = (p.xy / p.w) * 2;
	float2 uvSearch = float2(lightSize, lightSize) / orthoSize;

	float2 uv = coords.xy;
	float zReceiver = coords.z; // Assumed to be eye-space z in this code

	// STEP 1: blocker search
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	FindBlocker(shadowMap, avgBlockerDepth, numBlockers, uv, zReceiver, uvSearch, depthBias); // we are scaling down the search area as a TEMP fix for the grainy shaodws
	if (numBlockers < 1)
		//There are no occluders so early out (this saves filtering)
		return 1.0f;

	// STEP 2: penumbra size
	float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);
	float2 filterRadiusUV = penumbraRatio * uvSearch;

	// STEP 3: filtering
	int ditherIndex = Hash1(pixelLocation.x * 1750.8743 + pixelLocation.y * 9753.2198 + pixelLocation.z * 4930.9434) * DITHER_OFFSETS;
	return PCF_Filter(shadowMap, uv, zReceiver, filterRadiusUV, ditherIndex, depthBias);
}



uint GetCSMIndex(float depth, StructuredBuffer<DirectionalLightCascade> cascades)
{
	uint numCascades;
	uint stride;
	cascades.GetDimensions(numCascades, stride);
	for (uint i = 0; i < numCascades; i++)
	{
		DirectionalLightCascade c = cascades[i];
		float d = depth;
		float min = c.minDist;
		float max = c.maxDist;
		if (d > min && d <= max)
			return i;
	}
	
	return UINT_MAX;
}


float CalcCSMBias(uint ci, float3 normal, float3 lightDir)
{
	return DEPTH_BIAS + (0.0002 * ci) * dot(normal, -lightDir);
}

float4 CalcCSMSamplePoint(float4x4 viewProjection, float4 worldPosition)
{
	float4 pos = mul(viewProjection, worldPosition);
	pos.y = -pos.y; // flip v
	pos = pos / pos.w;
	pos.xy = pos.xy * 0.5 + 0.5; // map from -1:1 -> 0:1
	return pos;
}

float SampleCSMCascade(DirectionalLightCascade cascade, Texture2D shadowMap, Camera camera, DirectionalLight light, float4 worldPosition, float bias)
{
	float4 pos = CalcCSMSamplePoint(camera.ViewPorjection, worldPosition);
	return PCSSDirectional(shadowMap, pos, camera.InvProjection, light.size, (float3) worldPosition, bias);
}


#endif
