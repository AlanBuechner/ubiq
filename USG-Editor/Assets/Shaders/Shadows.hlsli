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
  float2(-0.000767, -0.000703),
  float2(0.000366, 0.004146),
  float2(0.005693, -0.007436),
  float2(-0.016392, 0.002914),
  float2(0.021965, 0.013939),
  float2(-0.009771, -0.036164),
  float2(-0.023433, 0.045285),
  float2(0.062517, -0.022952),
  float2(-0.077972, -0.032010),
  float2(0.044306, 0.094155),
  float2(0.037401, -0.120227),
  float2(-0.129453, 0.075465),
  float2(0.171861, 0.037284),
  float2(-0.117797, -0.166497),
  float2(-0.029344, 0.232285),
  float2(0.203107, -0.172368),
  float2(-0.300515, -0.011337),
  float2(0.239894, 0.236897),
  float2(-0.018874, -0.375176),
  float2(-0.265319, 0.320711),
  float2(0.454520, -0.063233),
  float2(-0.414765, -0.285701),
  float2(0.123451, 0.536447),
  float2(0.295326, -0.521568),
  float2(-0.618533, 0.200973),
  float2(0.640198, 0.291489),
  float2(-0.296883, -0.698077),
  float2(-0.271531, 0.769303),
  float2(0.772115, -0.411935),
  float2(-0.907103, -0.232899),
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
