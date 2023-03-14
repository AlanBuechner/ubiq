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
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	uint matID : MATID;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};

#section vertex

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 ViewPorjection;

	float3 Position;
	float3 rotation;
};

cbuffer MainCameraIndex
{
	uint mainCameraIndex;
};

ConstantBuffer<Camera> cameras[];


VS_Output main(VS_Input input)
{
	VS_Output output;
	Camera camera = cameras[mainCameraIndex];
	output.position = mul(camera.ViewPorjection, mul(input.transform, float4(input.position, 1)));
	output.uv = input.uv;
	output.normal = input.normal;
	output.tangent = input.tangent;
	output.matID = input.materialID;
	return output;
}

#section pixel

struct Material
{
	uint diffuse;
	uint normal;
	uint specular;
};

cbuffer DirectinalLight
{
	float3 direction;
	float3 color;
	float intensity;
};

ConstantBuffer<Material> materials[];
Texture2D<float4> textures[];
sampler s;

PS_Output main(PS_Input input)
{
	PS_Output output;
	
	Material mat = materials[input.matID];

	output.color = textures[mat.diffuse].Sample(s, input.uv);
	return output;
}

#section depth

struct Material
{
	uint diffuse;
	uint normal;
	uint specular;
};

ConstantBuffer<Material> materials[];
Texture2D<float4> textures[];
sampler s;

void main(PS_Input input)
{

}
