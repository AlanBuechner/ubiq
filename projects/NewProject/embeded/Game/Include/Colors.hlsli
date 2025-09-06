#ifndef COLORS_HLSLI
#define COLORS_HLSLI

#include "Common.hlsli"

// https://www.chilliant.com/rgb2hsv.html

float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R, G, B));
}

float3 RGBtoHCV(in float3 RGB)
{
	// Based on work by Sam Hocevar and Emil Persson
	float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
	float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	float H = abs((Q.w - Q.y) / (6 * C + EPSILON) + Q.z);
	return float3(H, C, Q.x);
}


float3 HSVtoRGB(in float3 HSV)
{
	float3 RGB = HUEtoRGB(HSV.x);
	return ((RGB - 1) * HSV.y + 1) * HSV.z;
}

float3 HSLtoRGB(in float3 HSL)
{
	float3 RGB = HUEtoRGB(HSL.x);
	float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
	return (RGB - 0.5) * C + HSL.z;
}

float3 RGBtoHSV(in float3 RGB)
{
	float3 HCV = RGBtoHCV(RGB);
	float S = HCV.y / (HCV.z + EPSILON);
	return float3(HCV.x, S, HCV.z);
}


float3 RGBtoHSL(in float3 RGB)
{
	float3 HCV = RGBtoHCV(RGB);
	float L = HCV.z - HCV.y * 0.5;
	float S = HCV.y / (1 - abs(L * 2 - 1) + EPSILON);
	return float3(HCV.x, S, L);
}



// https://gist.github.com/totallyRonja/8b9d571225f31c7a0d14872cf1478c85
static const float3x3 s_Lrgb2cone =
{
	0.412165612, 0.211859107, 0.0883097947,
	0.536275208, 0.6807189584, 0.2818474174,
	0.0514575653, 0.107406579, 0.6302613616,
};

static const float3x3 s_Cone2lab =
{
	+0.2104542553, +1.9779984951, +0.0259040371,
	+0.7936177850, -2.4285922050, +0.7827717662,
	+0.0040720468, +0.4505937099, -0.8086757660,
};

static const float3x3 s_Lab2cone =
{
	+4.0767416621, -1.2684380046, -0.0041960863,
	-3.3077115913, +2.6097574011, -0.7034186147,
	+0.2309699292, -0.3413193965, +1.7076147010,
};

static const float3x3 s_Cone2lrgb =
{
	1, 1, 1,
	+0.3963377774f, -0.1055613458f, -0.0894841775f,
	+0.2158037573f, -0.0638541728f, -1.2914855480f,
};

//conversion from linear srgb to oklab colorspace
float3 RGBtoOKLAB(float3 col)
{
	col = mul(col, s_Lrgb2cone);
	col = pow(col, 1.0 / 3.0);
	col = mul(col, s_Cone2lab);
	return col;
}


//conversion from oklab to linear srgb
float3 OKLABtoRGB(float3 col)
{
	col = mul(col, s_Cone2lrgb);
	col = col * col * col;
	col = mul(col, s_Lab2cone);
	return col;
}

#endif