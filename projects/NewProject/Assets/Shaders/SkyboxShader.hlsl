#section config
topology = triangle;

passes = { 
	main = {
		VS = vertex;
		PS = pixel;
	};
};

#section common
#pragma enable_d3d12_debug_symbols

struct VS_Input
{
	float4 position : POSITION;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float3 worldPosition : WORLDPOSITION;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	float4 color : SV_TARGET0;
};

float4x4 inverse(float4x4 input)
{
#define minor(a,b,c) determinant(float3x3(input.a, input.b, input.c))
	//determinant(float3x3(input._22_23_23, input._32_33_34, input._42_43_44))

	float4x4 cofactors = float4x4(
		minor(_22_23_24, _32_33_34, _42_43_44),
		-minor(_21_23_24, _31_33_34, _41_43_44),
		minor(_21_22_24, _31_32_34, _41_42_44),
		-minor(_21_22_23, _31_32_33, _41_42_43),

		-minor(_12_13_14, _32_33_34, _42_43_44),
		minor(_11_13_14, _31_33_34, _41_43_44),
		-minor(_11_12_14, _31_32_34, _41_42_44),
		minor(_11_12_13, _31_32_33, _41_42_43),

		minor(_12_13_14, _22_23_24, _42_43_44),
		-minor(_11_13_14, _21_23_24, _41_43_44),
		minor(_11_12_14, _21_22_24, _41_42_44),
		-minor(_11_12_13, _21_22_23, _41_42_43),

		-minor(_12_13_14, _22_23_24, _32_33_34),
		minor(_11_13_14, _21_23_24, _31_33_34),
		-minor(_11_12_14, _21_22_24, _31_32_34),
		minor(_11_12_13, _21_22_23, _31_32_33)
		);
#undef minor
	return transpose(cofactors) / determinant(input);
}

#section vertex

struct Camera
{
	float4x4 View;
	float4x4 Porjection;
	float4x4 InvPorjection;
	float4x4 ViewPorjection;

	float3 Position;
	float3 rotation;
};

RootConstant<uint> u_MainCameraIndex;

ConstantBuffer<Camera> u_Cameras[];

VS_Output main(VS_Input input)
{
	VS_Output output;
	Camera camera = u_Cameras[u_MainCameraIndex];
	output.position = input.position;
	output.worldPosition = normalize(mul(inverse(camera.ViewPorjection), input.position).xyz);
	return output;
}

#section pixel

Texture2D<float4> u_Texture;
StaticSampler s_TextureSampler = StaticSampler(repeat, repeat, anisotropic, anisotropic);

#include "Game/Include/common.hlsli"

[earlydepthstencil]
PS_Output main(PS_Input input)
{
	PS_Output output;

	float3 wp = input.worldPosition;

	float lon = -atan2(wp.z, wp.x);
	float lat = atan(wp.y / length(wp.xz));
	float2 uv = float2((lon + PI) / (2 * PI), 1 - (lat + (PI / 2)) / PI);

	output.color = u_Texture.SampleLevel(s_TextureSampler, uv, 0); // *0 + float4(uv, 0, 1);
	return output;
}
