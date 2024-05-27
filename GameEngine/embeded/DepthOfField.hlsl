#section config
topology = triangle;

passes = {
	CoC = {
		VS = vertex;
		PS = CoC;
	};
	expandCoC = {
		VS = vertex;
		PS = expandCoC;
	};
	blurCoC = {
		VS = vertex;
		PS = blurCoC;
	};
	bokehBlur = {
		VS = vertex;
		PS = bokehBlur;
	};
	composit = {
		VS = vertex;
		PS = composit;
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
	float2 uv : UV;
};

typedef VS_Output PS_Input;

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

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = input.position;
	output.uv = float2(input.position.x+1, 1-input.position.y)/2;
	return output;
}






#section CoC

struct PS_Output
{
	float4 COC : SV_TARGET0;
};

cbuffer RC_DepthLoc{
	uint depthLoc;
};

cbuffer RC_Radius{
	float radius;
};

cbuffer RC_FocalPlane{
	float focalPlane;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, point, point);

ConstantBuffer<Camera> camera;

float DepthNDCToView(float depth_ndc, float zNear, float zFar) 
{
	float2 projParams = float2(zFar / (zNear - zFar), zNear * zFar / (zNear - zFar));
	return projParams.y / (depth_ndc + projParams.x);
}

PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> depthTexture = textures[depthLoc];
	float depth = depthTexture.Sample(textureSampler, input.uv).r;

	float near = camera.Porjection._m32 / (camera.Porjection._m22 + 1.0);
	float far = camera.Porjection._m32 / (camera.Porjection._m22 - 1.0);

	depth = DepthNDCToView(depth, near, far);

	float nearBegin = max(0.0f, focalPlane - radius);
	float nearEnd = focalPlane;
	float farBegin = focalPlane;
	float farEnd = focalPlane + radius;

	float nearCOC = 0.0f;
	if (depth < nearEnd)
		nearCOC = 1.0f / (nearBegin - nearEnd) * depth + -nearEnd / (nearBegin - nearEnd);
	else if (depth < nearBegin)
		nearCOC = 1.0f;

	float farCOC = 1.0f;
	if (depth < farBegin)
		farCOC = 0.0f;
	else if (depth < farEnd)
		farCOC = 1.0f / (farEnd - farBegin) * depth + -farBegin / (farEnd - farBegin);

	if (depth >= 999.0f)
		farCOC = 1.0f;

	output.COC = float4(nearCOC, farCOC, 0, 1);
	return output;
}









#section expandCoC

struct PS_Output
{
	float4 COC : SV_TARGET0;
};

cbuffer RC_X {
	bool expandX;
};

cbuffer RC_Radius {
	float radius;
};

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];


PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> src = textures[srcLoc];

	uint2 srcTexelCount;
	src.GetDimensions(srcTexelCount.x, srcTexelCount.y);
	int numTexels = radius * srcTexelCount.y;

	float4 maxval = src.Load(int3(input.position.xy, 0));

	int2 dir = int2(expandX ? 1 : 0, expandX ? 0 : 1);
	for (int i = -numTexels; i < numTexels; i++)
	{
		float val = src.Load(int3(input.position.xy + dir * i, 0)).r;
		maxval.r = max(maxval.r, val);
	}

	output.COC = maxval;
	return output;
}








#section blurCoC

struct PS_Output
{
	float4 COC : SV_TARGET0;
};

cbuffer RC_X {
	bool blurX;
};

cbuffer RC_Radius {
	float radius;
};

cbuffer RC_SrcLoc
{
	uint srcLoc;
};

Texture2D<float4> textures[];

PS_Output main(PS_Input input)
{
	PS_Output output;

	Texture2D<float4> src = textures[srcLoc];

	uint2 srcTexelCount;
	src.GetDimensions(srcTexelCount.x, srcTexelCount.y);
	int numTexels = radius * srcTexelCount.y;
	
	float4 sum = float4(0, src.Load(int3(input.position.xy, 0)).g, 0, 1);

	int2 dir = int2(blurX ? 1 : 0, blurX ? 0 : 1);
	for (int i = -numTexels; i < numTexels; i++)
	{
		float val = src.Load(int3(input.position.xy + dir*i, 0)).r;
		sum.r += val;
	}

	output.COC = sum;
	output.COC.r /= (2 * numTexels) + 1;

	return output;
}







#section bokehBlur

// Circular Kernel from GPU Zen 'Practical Gather-based Bokeh Depth of Field' by Wojciech Sterna
static const float2 offsets[] =
{
	2.0f * float2(1.000000f, 0.000000f),
	2.0f * float2(0.707107f, 0.707107f),
	2.0f * float2(-0.000000f, 1.000000f),
	2.0f * float2(-0.707107f, 0.707107f),
	2.0f * float2(-1.000000f, -0.000000f),
	2.0f * float2(-0.707106f, -0.707107f),
	2.0f * float2(0.000000f, -1.000000f),
	2.0f * float2(0.707107f, -0.707107f),

	4.0f * float2(1.000000f, 0.000000f),
	4.0f * float2(0.923880f, 0.382683f),
	4.0f * float2(0.707107f, 0.707107f),
	4.0f * float2(0.382683f, 0.923880f),
	4.0f * float2(-0.000000f, 1.000000f),
	4.0f * float2(-0.382684f, 0.923879f),
	4.0f * float2(-0.707107f, 0.707107f),
	4.0f * float2(-0.923880f, 0.382683f),
	4.0f * float2(-1.000000f, -0.000000f),
	4.0f * float2(-0.923879f, -0.382684f),
	4.0f * float2(-0.707106f, -0.707107f),
	4.0f * float2(-0.382683f, -0.923880f),
	4.0f * float2(0.000000f, -1.000000f),
	4.0f * float2(0.382684f, -0.923879f),
	4.0f * float2(0.707107f, -0.707107f),
	4.0f * float2(0.923880f, -0.382683f),

	6.0f * float2(1.000000f, 0.000000f),
	6.0f * float2(0.965926f, 0.258819f),
	6.0f * float2(0.866025f, 0.500000f),
	6.0f * float2(0.707107f, 0.707107f),
	6.0f * float2(0.500000f, 0.866026f),
	6.0f * float2(0.258819f, 0.965926f),
	6.0f * float2(-0.000000f, 1.000000f),
	6.0f * float2(-0.258819f, 0.965926f),
	6.0f * float2(-0.500000f, 0.866025f),
	6.0f * float2(-0.707107f, 0.707107f),
	6.0f * float2(-0.866026f, 0.500000f),
	6.0f * float2(-0.965926f, 0.258819f),
	6.0f * float2(-1.000000f, -0.000000f),
	6.0f * float2(-0.965926f, -0.258820f),
	6.0f * float2(-0.866025f, -0.500000f),
	6.0f * float2(-0.707106f, -0.707107f),
	6.0f * float2(-0.499999f, -0.866026f),
	6.0f * float2(-0.258819f, -0.965926f),
	6.0f * float2(0.000000f, -1.000000f),
	6.0f * float2(0.258819f, -0.965926f),
	6.0f * float2(0.500000f, -0.866025f),
	6.0f * float2(0.707107f, -0.707107f),
	6.0f * float2(0.866026f, -0.499999f),
	6.0f * float2(0.965926f, -0.258818f),
};

struct PS_BokehOutput
{
	float4 near : SV_TARGET0;
	float4 far : SV_TARGET1;
};

cbuffer RC_SrcLoc{
	uint srcLoc;
};

cbuffer RC_Strength {
	float strength;
};

cbuffer RC_COC {
	uint cocLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);
StaticSampler pointSampler = StaticSampler(clamp, clamp, point, point);

PS_BokehOutput main(PS_Input input)
{
	PS_BokehOutput output;

	Texture2D<float4> src = textures[srcLoc];
	Texture2D<float4> coc = textures[cocLoc];

	uint2 srcTexelCount;
	src.GetDimensions(srcTexelCount.x, srcTexelCount.y);
	float2 srcPixelUVSize = 1.0 / srcTexelCount;

	// cocs
	float COCs[48];
	float pixelCOC = coc.Sample(pointSampler, input.uv).g;
	float COCsum = pixelCOC;
	for (uint i = 0; i < 48; i++)
	{
		float2 offset = offsets[i] * strength * srcPixelUVSize * pixelCOC;
		float farCOC = coc.Sample(pointSampler, input.uv + offset).g;
		COCs[i] = farCOC;
		COCsum += farCOC;
	}
	float avgCOC = max(COCsum / 49.0, 0.00001);

	// near blur
	float4 nearSum = src.Sample(textureSampler, input.uv);
	for (uint i = 0; i < 48; i++)
	{
		float2 offset = offsets[i] * strength * srcPixelUVSize;
		nearSum += src.Sample(textureSampler, input.uv + offset);
	}

	// far blur
	float4 farSum = src.Sample(textureSampler, input.uv);
	for (uint i = 0; i < 48; i++)
	{
		float2 offset = offsets[i] * strength * srcPixelUVSize * pixelCOC;
		float farCOC = COCs[i];
		farSum += src.Sample(textureSampler, input.uv + offset) * (farCOC / avgCOC);
	}

	output.near = nearSum/49.0;
	output.near.a = 1;
	output.far = farSum/49.0;
	output.far.a = 1;
	return output;
}




#section composit

struct PS_Output
{
	float4 color : SV_TARGET0;
};

cbuffer RC_SrcLoc{
	uint srcLoc;
};

cbuffer RC_NearLoc {
	uint nearLoc;
};

cbuffer RC_FarLoc {
	uint farLoc;
};

cbuffer RC_COC {
	uint cocLoc;
};

Texture2D<float4> textures[];
StaticSampler textureSampler = StaticSampler(clamp, clamp, point, point);

PS_Output main(PS_Input input)
{
	PS_Output output;

	float4 src = textures[srcLoc].Sample(textureSampler, input.uv);
	float4 near = textures[nearLoc].Sample(textureSampler, input.uv);
	float4 far = textures[farLoc].Sample(textureSampler, input.uv);
	float2 coc = textures[cocLoc].Sample(textureSampler, input.uv).rg;

	//float4 background = lerp(src, far, coc.g);
	float4 background = lerp(src, far, sqrt(coc.g));
	//float4 background = lerp(src, far/max(coc.g, 0.00001), coc.g);

	output.color = lerp(background, near, coc.r);
	output.color.a = 1;
	return output;
}
