#ifndef MATERIAL_HLSLI
#define MATERIAL_HLSLI

StaticSampler s_NormalSampler = StaticSampler(repeat, repeat, anisotropic, anisotropic);


float3 FixNormal(float3 n, float3 normal, float3 tangent, float3 bitangent)
{
	bool3 nan = isnan(tangent);
	float tSqrLength = dot(tangent, tangent);
	if (nan.x || nan.y || nan.z || tSqrLength < 0.1)
		return normal;

	float3x3 TBN = transpose(float3x3(normalize(tangent), normalize(bitangent), normalize(normal)));
	return normalize(mul(TBN, n));
}

float3 SampleNormalMap(Texture2D normalMap, float2 uv)
{
	return normalize(normalize(normalMap.Sample(s_NormalSampler, uv).rgb) * 2 - 1);
}

#endif
