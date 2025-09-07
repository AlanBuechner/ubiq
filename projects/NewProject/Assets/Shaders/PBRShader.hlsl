#section config
topology = triangle;

passes = { 
	lit = {
		VS =
vertex;
		PS = pixel;
	};
	depth = {
		VS = vertex;
		PS = depth;
	};
	directionalShadowMap = {
		VS = vertex;
		PS = directionalShadowMap;
		blendMode = none;
		//depthTest = Greater;
		//cullMode = back;
	};
};

material = {
	diffuse = textureID(white);
	normal = textureID(normal);
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
	uint materialID : MATID;
};

struct PS_Input : VS_Output
{
};

struct Depth_PS_Input : VS_Output
{
};

#include "Engine/Shaders/Include/Camera.hlsli"












#section vertex


RootConstant<uint> u_MainCameraIndex;

// Material struct is generated automaticly by the data defined in the config section
ConstantBuffer<Material> u_Materials[] : register(space0);
ConstantBuffer<Camera> u_Cameras[] : register(space1);


VS_Output main(VS_Input input)
{
	Material mat = u_Materials[input.materialID];
	
	Camera camera = u_Cameras[u_MainCameraIndex];
	float4x4 mvp = mul(camera.ViewPorjection, input.transform);
	float4x4 mv = mul(camera.View, input.transform);

	VS_Output output;
	output.position = mul(mvp, float4(input.position, 1));
	output.worldPosition = mul(input.transform, float4(input.position, 1));
	float4 cameraSpacePosition = mul(mv, float4(input.position, 1));
	output.depth = cameraSpacePosition.z;
	output.uv = input.uv * mat.tiling;
	if (mat.flipV)
		output.uv.y = 1 - output.uv.y;

	output.normal = normalize(mul((float3x3) input.transform, input.normal));
	output.tangent = normalize(mul((float3x3) input.transform, input.tangent));

	output.bitangent = normalize(cross(output.normal, output.tangent));

	output.PToC = camera.Position - output.worldPosition.xyz;
	
	output.materialID = input.materialID;

	return output;
}



#section pixel


#include "RenderingUtilsModule/Shaders/Include/Common.hlsli"
#include "RenderingUtilsModule/Shaders/Include/PBR.hlsli"
#include "RenderingUtilsModule/Shaders/Include/Shadows.hlsli"
#include "RenderingUtilsModule/Shaders/Include/Material.hlsli"


struct PS_Output
{
	float4 color : SV_TARGET0;
};

// Material struct is generated automaticly by the data defined in the config section
ConstantBuffer<Material> u_Materials[] : register(b0, space0);
ConstantBuffer<Camera> u_Cameras[] : register(b0, space1);

RootConstant<uint> u_UseDirLight : register(b0, space2);
ConstantBuffer<DirectionalLight> u_DirLight : register(b1, space2);
StructuredBuffer<DirectionalLightCascade> u_Cascades : register(t0, space0);


Texture2D<float4> u_Textures[] : register(t1, space0);
StaticSampler s_TextureSampler = StaticSampler(repeat, repeat, anisotropic, anisotropic);



float SampleCSM(float4 worldPosition, float3 normal, float depth)
{
	if (!u_UseDirLight)
		return 0.0;
	
	uint ci = GetCSMIndex(depth, u_Cascades);

	if (ci == UINT_MAX)
		return 0.0;

	DirectionalLightCascade c = u_Cascades[ci];
	Camera shadowCamera = u_Cameras[c.camera];
	Texture2D shadowMap = u_Textures[NonUniformResourceIndex(c.texture)];
	float bias = CalcCSMBias(ci, normal, u_DirLight.direction);
			
	return SampleCSMCascade(c, shadowMap, shadowCamera, u_DirLight, worldPosition, bias);
		
	//shadowAmount = MomentShadow(shadowMap, pos, input.position.z, bias);
	//shadowAmount = PCSSDirectional(shadowMap, pos, shadowCamera.InvProjection, u_DirLight.size, (float3) input.worldPosition, bias);
	//shadowAmount = HardShadow(shadowMap, pos);
}




[earlydepthstencil]
PS_Output main(PS_Input input)
{
	Material mat = u_Materials[input.materialID];
	
	PS_Output output;

	float3 viewDirection = normalize(input.PToC);
	float2 uv = input.uv;
	
	float4 diffuse = u_Textures[mat.diffuse].Sample(s_TextureSampler, uv);
	float3 normal = SampleNormalMap(u_Textures[mat.normal], uv);
	normal = FixNormal(normal, input.normal, input.tangent, input.bitangent);
	float roughness = u_Textures[mat.roughness].Sample(s_TextureSampler, uv).r;
	float metallic = u_Textures[mat.metal].Sample(s_TextureSampler, uv).b;
	float ao = u_Textures[mat.ao].Sample(s_TextureSampler, uv).r;
	
	
	
	float3 lo = float3(0, 0, 0);

	// directinal light
	
	{
		float shadowAmount = SampleCSM(input.worldPosition, input.normal, input.depth);
		if (shadowAmount != 0)
			lo += PBR(diffuse.rgb, u_DirLight, viewDirection, normal, roughness, metallic) * shadowAmount;
	}

	float3 ambiant = float3(0.5, 0.5, 0.5) * diffuse.rgb;

	output.color = float4((ambiant + lo) * ao, diffuse.a);

	return output;
}








#section depth


void main(Depth_PS_Input input)
{

}









#section directionalShadowMap

#include "RenderingUtilsModule/Shaders/Include/Shadows.hlsli"


struct PS_Output
{
	float4 color : SV_TARGET0;
	float depth : SV_DEPTH;
};

[earlydepthstencil]
PS_Output main(Depth_PS_Input input)
{
	PS_Output output;

	float d = input.position.z;

	//float4 moments;
	//moments.r = d;
	//moments.g = d*d;
	//moments.b = d*d*d;
	//moments.a = d*d*d*d;

	//output.color = CompressMoments(moments);

	output.color = float4(d, 0, 0, 0);
	output.depth = d;

	return output;
}
