#define DEPTH_BIAS (0.0005)

float HardShadow(Texture2D shadowMap, float4 coords)
{
	float depthSample = shadowMap.Sample(P_s, coords.xy).r + DEPTH_BIAS;
	return (depthSample < coords.z ? 0 : 1);
}

#define BLOCKER_SEARCH_NUM_SAMPLES 32
#define PCF_NUM_SAMPLES 32

static float2 poissonDisk[32] = {
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

//static float2 poissonDisk[16] = {
//	float2(-0.94201624, -0.39906216),
//	float2(0.94558609, -0.76890725),
//	float2(-0.094184101, -0.92938870),
//	float2(0.34495938, 0.29387760),
//	float2(-0.91588581, 0.45771432),
//	float2(-0.81544232, -0.87912464),
//	float2(-0.38277543, 0.27676845),
//	float2(0.97484398, 0.75648379),
//	float2(0.44323325, -0.97511554),
//	float2(0.53742981, -0.47373420),
//	float2(-0.26496911, -0.41893023),
//	float2(0.79197514, 0.19090188),
//	float2(-0.24188840, 0.99706507),
//	float2(-0.81409955, 0.91437590),
//	float2(0.19984126, 0.78641367),
//	float2(0.14383161, -0.14100790)
//};

float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
{
	return (zReceiver - zBlocker) / zBlocker;
}

void FindBlocker(Texture2D shadowMap, out float avgBlockerDepth, out float numBlockers, float2 uv, float zReceiver, float2 searchSize)
{
	//This uses similar triangles to compute what
	float blockerSum = 0;
	numBlockers = 0;

	for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
	{
		float2 offset = poissonDisk[i] * searchSize;
		float shadowMapDepth = shadowMap.SampleLevel(P_s, uv + offset, 0).r + DEPTH_BIAS;
		if (shadowMapDepth < zReceiver) {
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
	avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter(Texture2D shadowMap, float2 uv, float zReceiver, float2 filterRadiusUV)
{
	float sum = 0.0f;
	for (int i = 0; i < PCF_NUM_SAMPLES; ++i)
	{
		float2 offset = poissonDisk[i] * filterRadiusUV;
		float depthSample = shadowMap.Sample(P_s, uv + offset).r + DEPTH_BIAS;
		sum += (depthSample < zReceiver ? 0 : 1);
		//sum += shadowMap.SampleCmpLevelZero(P_s, uv + offset, zReceiver).r;
	}
	return sum / PCF_NUM_SAMPLES;
}

float PCSSDirectional(Texture2D shadowMap, float4 coords, float4x4 ortho, float lightSize)
{
	float4 p = mul(ortho, float4(1, 1, 1, 1));
	float2 orthoSize = (p.xy / p.w) * 2;
	float2 uvSearch = float2(lightSize, lightSize) / orthoSize;

	float2 uv = coords.xy;
	float zReceiver = coords.z; // Assumed to be eye-space z in this code

	// STEP 1: blocker search
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	FindBlocker(shadowMap, avgBlockerDepth, numBlockers, uv, zReceiver, uvSearch); // we are scaling down the search area as a TEMP fix for the grainy shaodws
	if (numBlockers < 1)
		//There are no occluders so early out (this saves filtering)
		return 1.0f;

	// STEP 2: penumbra size
	float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);
	float2 filterRadiusUV = penumbraRatio * uvSearch;

	// STEP 3: filtering
	return PCF_Filter(shadowMap, uv, zReceiver, filterRadiusUV);
}
