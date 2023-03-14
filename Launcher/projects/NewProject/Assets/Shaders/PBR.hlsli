static const float PI = 3.14159265f;

// fresnel shlick function
float3 F_Schlick(float3 f0, float HdotV)
{
	return f0 + ((float3(1, 1, 1) - f0) * pow(float3(1, 1, 1) - HdotV, 5.0));
}

// specular D
float D(float a, float NdotH)
{
	float numerator = a * a;

	float denominator = PI * pow(((NdotH * NdotH) * ((a * a) - 1)) + 1, 2);
	denominator = max(denominator, 0.000001);

	return numerator / denominator;
}

float G1(float a, float NdotX)
{
	float k = a / 2.0;

	float denom = (NdotX * (1 - k)) + k;
	denom = max(denom, 0.000001);

	return NdotX / denom;
}

float G(float a, float NdotV, float NdotL)
{
	return G1(a, NdotV) * G1(a, NdotL);
}


float3 PBR(float3 color, float3 lightDirection, float3 lightColor, float intensity, float3 viewDirection, float3 normal, float roughness, float metallic, float3 baseReflectivity)
{
	// lighting
	float3 dir = normalize(-lightDirection);

	float3 H = normalize(viewDirection + dir); // half way vector

	float NdotV = max(dot(normal, viewDirection), 0.000001);
	float NdotL = max(dot(normal, dir), 0.000001);
	float NdotH = max(dot(normal, H), 0.0);
	float HdotV = max(dot(H, viewDirection), 0.0);

	float3 ks = F_Schlick(baseReflectivity, HdotV); // how much energy is contributed to specular
	float3 kd = (float3(1, 1, 1) - ks) * (1 - metallic); // how much energy is contribited to diffuse. the remaining amount of energy from after specular minus the energy ubsorbed by the metal

	float3 ctn = D(roughness, NdotH) * G(roughness, NdotV, NdotL) * ks;
	float ctd = 4.0 * NdotV * NdotL;
	ctd = max(ctd, 0.000001);

	float3 BRDF = (kd * color / PI) + (ctn / ctd);

	return BRDF * lightColor * intensity * NdotL;
}
