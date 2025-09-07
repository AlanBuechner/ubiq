#ifndef LIGHTING_COMMON_HLSLI
#define LIGHTING_COMMON_HLSLI



struct DirectionalLight
{
	float3 direction;
	float3 color;
	float intensity;
	float size;
};



struct DirectionalLightCascade
{
	uint camera;
	uint texture;
	uint tWidth;
	uint tHeight;
	float minDist;
	float maxDist;
};


#endif
