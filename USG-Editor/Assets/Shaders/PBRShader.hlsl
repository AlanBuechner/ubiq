#section config
topology = triangle;

passes = { 
	main = {
		VS = vertex;
		PS = pixel; 
	};
	depth = {
		VS = vertex;
		PS = depth;
	};
};

#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float3 position : POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;

	// semantics starting with "I_" are per instance data
	float4x4 transform : I_TRANSFORM;
	uint materialID : I_MATID;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL; // world space normal
	float3x3 TBN : TBN; // tbn frame
	float3 CToP : CTOP; // direction from the camera to the point
	uint matID : MATID;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};

// common structions
struct Material
{
	// texture indexes for the material properties
	uint diffuse;
	uint normal;
	uint roughness;
	uint ao;
	uint metal;
	uint disp;
};

struct DirectionalLight
{
	float3 direction;
	float3 color;
	float intensity;
};

#section vertex

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 ViewPorjection;

	float3 Position;
	float3 Rotation;
};

cbuffer MainCameraIndex
{
	uint mainCameraIndex;
};

ConstantBuffer<Camera> cameras[];


VS_Output main(VS_Input input)
{
	Camera camera = cameras[mainCameraIndex];
	float4x4 mvp = mul(camera.ViewPorjection, input.transform);
	float4x4 mv = mul(camera.View, input.transform);

	VS_Output output;
	output.position = mul(mvp, float4(input.position, 1));
	output.uv = input.uv;

	output.normal = normalize(mul((float3x3)input.transform, input.normal));
	float3 tangent = normalize(mul((float3x3)input.transform, input.tangent));
	//tangent = normalize(tangent - dot(tangent, output.normal) * output.normal);

	float3 bitangent = normalize(cross(output.normal, tangent));
	output.TBN = transpose(float3x3(tangent, bitangent, output.normal));

	output.CToP = normalize(camera.Position - output.position.xyz);

	output.matID = input.materialID;
	return output;
}

#section pixel

ConstantBuffer<DirectionalLight> DirLight;

ConstantBuffer<Material> materials[];
Texture2D<float4> textures[];
sampler s;

static const float PI = 3.14159265f;

// fresnel shlick function
float3 F_Schlick(float3 f0, float HdotV) 
{
	return f0 + (float3(1, 1, 1) - f0) * pow(float3(1, 1, 1) - HdotV, 5.0);
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

PS_Output main(PS_Input input)
{
	PS_Output output;
	
	Material mat = materials[input.matID];

	float4 diffuse = textures[mat.diffuse].Sample(s, input.uv);
	float3 normal = normalize(textures[mat.normal].Sample(s, input.uv).xyz * 2 - 1);
	float roughness = textures[mat.roughness].Sample(s, input.uv).r;
	float metallic = textures[mat.metal].Sample(s, input.uv).b;
	float ao = textures[mat.ao].Sample(s, input.uv).r;

	normal = normalize(mul(input.TBN, normal));
	float3 baseReflectivity = lerp(float3(0,0,0), diffuse.rgb, metallic);

	float3 lo = float3(0,0,0);

	// directinal light
	if(true) {
		float3 dir = normalize(-DirLight.direction);

		float3 H = normalize(input.CToP + dir); // half way vector

		float NdotV = max(dot(normal, input.CToP), 0.000001);
		float NdotL = max(dot(normal, dir), 0.000001);
		float NdotH = max(dot(normal, H), 0.0);
		float HdotV = max(dot(H, input.CToP), 0.0);

		float3 ks = F_Schlick(baseReflectivity, HdotV); // how much energy is contributed to specular
		float3 kd = (float3(1, 1, 1) - ks) * (1 - metallic); // how much energy is contribited to diffuse. the remaining amount of energy from after specular minus the energy ubsorbed by the metal

		float3 ctn = D(roughness, NdotH) * G(roughness, NdotV, NdotL) * ks;
		float ctd = 4.0 * NdotV * NdotL;
		ctd = max(ctd, 0.000001);

		float3 BRDF = (kd * diffuse.rgb / PI) + (ctn / ctd);

		lo += BRDF * DirLight.color * 5 * NdotL;
	}

	float3 ambiant = float3(0.3, 0.3, 0.3) * diffuse.rgb;

	output.color = float4((ambiant + lo) * ao, diffuse.a);
	return output;
}

#section depth

ConstantBuffer<Material> materials[];
Texture2D<float4> textures[];
sampler s;

void main(PS_Input input)
{

}
