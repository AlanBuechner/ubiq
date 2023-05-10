#section config
topology = triangle;

passes = { 
	lit = {
		VS = vertex;
		PS = pixel;
	};
	depth = {
		VS = vertex;
		PS = depth;
	};
};

material = {
	diffuse = textureID(white);
	normal = textureID(blue);
	roughness = textureID(white);
	ao = textureID(white);
	metal = textureID(black);
	disp = textureID(black);
	parallax = textureID(black);
	tiling = float(1);
	useParallax = bool(false);
	invertParallax = bool(false);
	flipV = bool(false);
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
	float3 PToC : PTOC; // direction from the camera to the point
	uint matID : MATID;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};

// common structures

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

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 InvProjection;
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

ConstantBuffer<Camera> cameras[] : register(space0);
// Material struct is generated automaticly by the data defined in the config section
ConstantBuffer<Material> materials[]: register(space1);


VS_Output main(VS_Input input)
{
	Material mat = materials[input.materialID];

	Camera camera = cameras[mainCameraIndex];
	float4x4 mvp = mul(camera.ViewPorjection, input.transform);
	float4x4 mv = mul(camera.View, input.transform);

	VS_Output output;
	output.position = mul(mvp, float4(input.position, 1));
	output.worldPosition = mul(input.transform, float4(input.position, 1));
	float4 cameraSpacePosition = mul(mv, float4(input.position, 1));
	output.depth = -cameraSpacePosition.z;
	output.uv = input.uv * mat.tiling;
	if (mat.flipV)
		output.uv.y = 1 - output.uv.y;

	output.normal = normalize(mul((float3x3)input.transform, input.normal));
	output.tangent = normalize(mul((float3x3)input.transform, input.tangent));

	output.bitangent = normalize(cross(output.normal, output.tangent));

	output.PToC = camera.Position - output.worldPosition.xyz;

	output.matID = input.materialID;

	return output;
}

#section pixel

ConstantBuffer<DirectionalLight> DirLight;
#define NUM_CASCADES 5
cbuffer Cascades
{
	DirectionalLightCascade cascades[NUM_CASCADES];
};

ConstantBuffer<Camera> cameras[] : register(space1);
// Material struct is generated automaticly by the data defined in the config section
ConstantBuffer<Material> materials[]: register(space2);

Texture2D<float4> textures[] : register(space0);
StaticSampler textureSampler = StaticSampler(repeat, repeat, anisotropic, anisotropic);

// P_ denotes sampler as having point filtering
StaticSampler shadowSampler = StaticSampler(clamp, clamp, point, point);

#include "PBR.hlsli"
#include "Shadows.hlsli"

float SampleParallax(uint textureID, bool invert, float2 uv)
{
	if (invert) return 1.0 - textures[textureID].Sample(textureSampler, uv).r;
	else return textures[textureID].Sample(textureSampler, uv).r;
}

[earlydepthstencil]
PS_Output main(PS_Input input)
{
	PS_Output output;
	
	Material mat = materials[input.matID];

	float3 viewDirection = normalize(input.PToC);
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

	float4 diffuse = textures[mat.diffuse].Sample(textureSampler, uv);
	float3 normal = normalize(normalize(textures[mat.normal].Sample(textureSampler, uv).rgb) * 2 - 1);
	float roughness = textures[mat.roughness].Sample(textureSampler, uv).r;
	float metallic = textures[mat.metal].Sample(textureSampler, uv).b;
	float ao = textures[mat.ao].Sample(textureSampler, uv).r;

	bool3 nan = isnan(input.tangent);
	if (nan.x || nan.y || nan.z)
	{
		normal = input.normal;
	}
	else
	{
		float3x3 TBN = transpose(float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal)));
		normal = normalize(mul(TBN, normal));
	}

	float3 lo = float3(0,0,0);

	// directinal light
	if(true) 
	{
		// depth testing
		uint numCascades = NUM_CASCADES;
		//uint stride;
		//cascades[cascadeIndex].GetDimensions(numCascades, stride);

		uint ci = numCascades;
		for (uint i = 0; i < numCascades; i++)
		{
			DirectionalLightCascade c = cascades[i];
			float d = input.depth;
			float min = c.minDist;
			float max = c.maxDist;
			if (d > min && d <= max)
			{
				ci = i;
				break;
			}
		}

		float shadowAmount = 1;
		if (ci < numCascades)
		{
			DirectionalLightCascade c = cascades[ci];
			Camera shadowCamera = cameras[c.camera];

			float4 pos = mul(shadowCamera.ViewPorjection, input.worldPosition);
			pos.y = -pos.y; // flip v
			pos = pos / pos.w;
			pos.xy = pos.xy * 0.5 + 0.5;

			shadowAmount = PCSSDirectional(textures[c.texture], shadowSampler, pos, shadowCamera.InvProjection, DirLight.size, (float3)input.worldPosition);
			//shadowAmount = HardShadow(textures[c.texture], shadowSampler, pos);
		}

		if(shadowAmount != 0)
		{
			//lo += float3(shadowAmount, shadowAmount, shadowAmount);
			lo += PBR(diffuse.rgb, DirLight.direction, DirLight.color, DirLight.intensity, viewDirection, normal, roughness, metallic) * shadowAmount;
		}
	}

	float3 ambiant = float3(0.5, 0.5, 0.5) * diffuse.rgb;

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
