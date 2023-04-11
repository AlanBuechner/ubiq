#define DEPTH_BIAS (0.0005)

float HardShadow(Texture2D shadowMap, sampler s, float4 coords)
{
	float depthSample = shadowMap.Sample(s, coords.xy).r + DEPTH_BIAS;
	return (depthSample < coords.z ? 0 : 1);
}

#define DITHER_OFFSETS 32
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

static float2 ditherOffsets[DITHER_OFFSETS] = {
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
	/*float2(-0.466668, 0.079521),
	float2(-0.249586, 0.520497),
	float2(0.025071, 0.335448),
	float2(0.063213, -0.921439),
	float2(-0.124725, 0.863670),
	float2(0.861620, 0.441905),
	float2(-0.431413, 0.477069),
	float2(0.279958, -0.291903),
	float2(0.375723, -0.668052),
	float2(-0.119791, 0.760150),
	float2(0.658402, -0.339326),
	float2(-0.542064, 0.786745),
	float2(-0.299280, 0.373340),
	float2(0.912936, 0.177280),
	float2(0.314608, 0.717353),
	float2(-0.120880, 0.847940),
	float2(-0.203127, 0.629534),
	float2(0.368437, 0.821944),
	float2(-0.035019, -0.568350),
	float2(0.900505, 0.840256),
	float2(-0.704680, 0.762124),
	float2(0.282161, -0.136093),
	float2(0.239193, -0.437881),
	float2(0.572004, -0.385084),
	float2(-0.105933, -0.547787),
	float2(-0.624934, -0.447531),
	float2(0.112888, -0.166997),
	float2(-0.660786, 0.813608),
	float2(-0.793658, -0.747849),
	float2(-0.009112, 0.520950),
	float2(0.969503, 0.870008),
	float2(0.368890, -0.233623),*/
};

float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
{
	return (zReceiver - zBlocker) / zBlocker;
}

void FindBlocker(Texture2D shadowMap, sampler s, out float avgBlockerDepth, out float numBlockers, float2 uv, float zReceiver, float2 searchSize)
{
	//This uses similar triangles to compute what
	float blockerSum = 0;
	numBlockers = 0;

	for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
	{
		float2 offset = poissonDisk[i] * searchSize;
		float shadowMapDepth = shadowMap.SampleLevel(s, uv + offset, 0).r + DEPTH_BIAS;
		if (shadowMapDepth < zReceiver) {
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
	avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter(Texture2D shadowMap, sampler s, float2 uv, float zReceiver, float2 filterRadiusUV, int ditherIndex)
{
	float sum = 0.0f;
	for (int i = 0; i < PCF_NUM_SAMPLES; ++i)
	{
		float2 ditherOffset = ditherOffsets[(ditherIndex + i) % DITHER_OFFSETS] * (3.141592654 / PCF_NUM_SAMPLES); // add dithering
		//float2 ditherOffset = ditherOffsets[(ditherIndex + i) % DITHER_OFFSETS]; // add dithering
		float2 offset = (poissonDisk[i] + ditherOffset) * filterRadiusUV;
		float depthSample = shadowMap.Sample(s, uv + offset).r + DEPTH_BIAS;
		sum += (depthSample < zReceiver ? 0 : 1);
	}
	return sum / PCF_NUM_SAMPLES;
}

float hash1(uint n)
{
	// integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
	n = n * (n * n * 15731U + 789221U) + 1376312589U;
	return float(n & uint(0x7fffffffU)) / float(0x7fffffff);
}

float PCSSDirectional(Texture2D shadowMap, sampler s, float4 coords, float4x4 ortho, float lightSize, float3 pixelLocation)
{
	float4 p = mul(ortho, float4(1, 1, 1, 1));
	float2 orthoSize = (p.xy / p.w) * 2;
	float2 uvSearch = float2(lightSize, lightSize) / orthoSize;

	float2 uv = coords.xy;
	float zReceiver = coords.z; // Assumed to be eye-space z in this code

	// STEP 1: blocker search
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	FindBlocker(shadowMap, s, avgBlockerDepth, numBlockers, uv, zReceiver, uvSearch); // we are scaling down the search area as a TEMP fix for the grainy shaodws
	if (numBlockers < 1)
		//There are no occluders so early out (this saves filtering)
		return 1.0f;

	// STEP 2: penumbra size
	float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);
	float2 filterRadiusUV = penumbraRatio * uvSearch;

	// STEP 3: filtering
	int ditherIndex = hash1(pixelLocation.x * 1750.8743 + pixelLocation.y* 9753.2198 + pixelLocation.z* 4930.9434) * DITHER_OFFSETS;
	return PCF_Filter(shadowMap, s, uv, zReceiver, filterRadiusUV, ditherIndex);
}
