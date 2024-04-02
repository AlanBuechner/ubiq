static const float PI = 3.14159265f;

// fresnel shlick function
float3 F_Schlick(float3 f0, float HdotV)
{
	return f0 + ((float3(1, 1, 1) - f0) * pow(float3(1, 1, 1) - HdotV, 5.0));
}

float3 F_Schlick(float3 f0, float HdotV, float roughness)
{
	float r = 1 - roughness;
	return f0 + ((max(float3(r, r, r), f0) - f0) * pow(float3(1, 1, 1) - HdotV, 5.0));
}

float D_GGX(float roughness, float NdotH) {
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float G1_Smith(float a, float NdotX)
{
	a = a + 1;
	float k = (a*a) / 8.0;

	float denom = (NdotX * (1 - k)) + k;
	denom = max(denom, 0.000001);

	return NdotX / denom;
}

float G_Smith(float a, float NdotV, float NdotL)
{
	return G1_Smith(a, NdotV) * G1_Smith(a, NdotL);
}


float3 PBR(float3 color, float3 lightDirection, float3 lightColor, float intensity, float3 viewDirection, float3 normal, float roughness, float metallic)
{
	float reflectance = 0.04;
	//float3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + color * metallic;
	float3 f0 = lerp(float3(reflectance, reflectance, reflectance), color, metallic);
	
	//float reflectance = 1.635;
	//float3 ior = float3(reflectance, reflectance, reflectance);
	//float3 f0 = pow((ior - 1) / (ior + 1), 2);

	// lighting
	float3 dir = normalize(-lightDirection);

	float3 H = normalize(viewDirection + dir); // half way vector

	float NdotV = abs(dot(normal, viewDirection)) + 1e-5;
	float NdotL = clamp(dot(normal, dir), 0.0, 1.0);
	float NdotH = clamp(dot(normal, H), 0.0, 1.0);
	float LdotH = clamp(dot(dir, H), 0.0, 1.0);

	float3 ks = F_Schlick(f0, LdotH); // how much energy is contributed to specular
	float3 kd = (float3(1, 1, 1) - ks) * (1 - metallic); // how much energy is contribited to diffuse. the remaining amount of energy from after specular minus the energy ubsorbed by the metal

	// ct = Cook Torrance
	float3 ctn = D_GGX(roughness, NdotH) * G_Smith(roughness, NdotV, NdotL) * ks;
	float ctd = 4.0 * NdotV * NdotL;
	ctd = max(ctd, 0.000001);

	float3 lambert = (kd * color / PI);
	float3 BRDF = lambert + (ctn / ctd);

	return BRDF * lightColor * intensity * NdotL;
}
