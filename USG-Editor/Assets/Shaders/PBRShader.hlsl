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
	float4 worldPosition : WORLD_POSITION;
	float depth : DEPTH; // the depth into the scene
	float2 uv : UV;
	float3 normal : NORMAL; // world space normal
	float3 tangent : TANGENT; // world space normal
	float3 bitangent : BITANGENT; // world space normal
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
	uint parallax;
	bool useParallax;
	bool invertParallax;
};

struct DirectionalLight
{
	float3 direction;
	float3 color;
	float intensity;
};

struct DirectionalLightCascade
{
	uint camera;
	uint texture;
	float minDist;
	float maxDist;
};

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 ViewPorjection;

	float3 Position;
	float3 Rotation;
};

#section vertex

// RC_ defines the cbuffer as a root constant
cbuffer RC_MainCameraIndex
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
	output.worldPosition = mul(input.transform, float4(input.position, 1));
	float4 cameraSpacePosition = mul(mv, float4(input.position, 1));
	output.depth = -cameraSpacePosition.z;
	output.uv = input.uv;

	output.normal = normalize(mul((float3x3)input.transform, input.normal));
	output.tangent = normalize(mul((float3x3)input.transform, input.tangent));

	output.bitangent = normalize(cross(output.normal, output.tangent));

	output.CToP = camera.Position - output.position.xyz;

	output.matID = input.materialID;
	return output;
}

#section pixel

//cbuffer RC_CascadeIndex
//{
//	uint cascadeIndex;
//};

ConstantBuffer<DirectionalLight> DirLight;
#define NUM_CASCADES 5
cbuffer Cascades
{
	DirectionalLightCascade cascades[NUM_CASCADES];
}[];

ConstantBuffer<Camera> cameras[] : register(space1);
ConstantBuffer<Material> materials[]: register(space2);

Texture2D<float4> textures[] : register(space0);
sampler s;

// P_ denotes sampler as having point filtering
sampler P_s;

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

float SampleParallax(uint textureID, bool invert, float2 uv)
{
	if (invert) return 1.0 - textures[textureID].Sample(s, uv).r;
	else return textures[textureID].Sample(s, uv).r;
}

PS_Output main(PS_Input input)
{
	PS_Output output;
	
	Material mat = materials[input.matID];

	float3 viewDirection = normalize(input.CToP);
	float2 uv = input.uv;

	if (mat.useParallax)
	{
		float heightScale = 0.05;
		const float minLayers = 8.0;
		const float maxLayers = 64.0;
		float numLayers = lerp(minLayers, maxLayers, abs(dot(input.normal, viewDirection)));

		float layerDepth = 1.0 / numLayers;
		float currLayerDepth = 0.0;

		float2 s = viewDirection.xy / viewDirection.z * heightScale;
		float2 deltaUV = s / numLayers;

		float2 UVs = uv;
		float currDepth = SampleParallax(mat.parallax, mat.invertParallax, UVs);

		while (currLayerDepth < currDepth)
		{
			UVs -= deltaUV;
			currDepth = SampleParallax(mat.parallax, mat.invertParallax, UVs);
			currLayerDepth += layerDepth;
		}

		float2 prevTexCoords = UVs + deltaUV;
		float afterDepth = currDepth - currLayerDepth;
		float beforeDepth = SampleParallax(mat.parallax, mat.invertParallax, prevTexCoords) - currLayerDepth + layerDepth;
		float weight = afterDepth / (afterDepth - beforeDepth);
		uv = prevTexCoords * weight + UVs * (1.0f - weight);

		// Get rid of anything outside the normal range
		//if (uv.x > 1.0 || uv.y > 1.0 || uv.x < 0.0 || uv.y < 0.0)
			//discard;
	}

	float4 diffuse = textures[mat.diffuse].Sample(s, uv);
	float3 normal = normalize(textures[mat.normal].Sample(s, uv).xyz * 2 - 1);
	float roughness = textures[mat.roughness].Sample(s, uv).r;
	float metallic = textures[mat.metal].Sample(s, uv).b;
	float ao = textures[mat.ao].Sample(s, uv).r;

	float3x3 TBN = transpose(float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal)));
	normal = normalize(mul(TBN, normal));
	float3 baseReflectivity = lerp(float3(0,0,0), diffuse.rgb, metallic);

	float3 lo = float3(0,0,0);

	// directinal light
	if(true) 
	{
		// depth testing
		uint numCascades = NUM_CASCADES;
		//uint stride;
		//cascades[cascadeIndex].GetDimensions(numCascades, stride);

		uint ci = 0;
		for (uint i = 0; i < numCascades; i++)
		{
			DirectionalLightCascade c = cascades[i];
			if (input.depth > c.minDist && input.depth <= c.maxDist)
			{
				ci = i;
				break;
			}
		}

		DirectionalLightCascade c = cascades[ci];
		Camera shadowCamera = cameras[c.camera];

		float4 pos = mul(shadowCamera.ViewPorjection, input.worldPosition);
		pos.y = - pos.y; // flip v
		pos = pos / pos.w;

		float depthSample = textures[c.texture].Sample(P_s, pos.xy * 0.5 + 0.5).r;

		if (!(depthSample+0.01 < pos.z))
		{
			// lighting
			float3 dir = normalize(-DirLight.direction);

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

			float3 BRDF = (kd * diffuse.rgb / PI) + (ctn / ctd);

			lo += BRDF * DirLight.color * 5 * NdotL;
		}
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
