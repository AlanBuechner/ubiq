#section config
topology = triangle;

passes = { 
	panel = {
		VS = vertex;
		PS = pixel;
		instanceStreams = [1,2];
	};
};









#section common
#pragma enable_d3d12_debug_symbols

#define FLT_MAX (3.402823466e+38)


struct VS_Input
{
	float2 position : POSITION_0_;
	
	float4 bounds : BOUNDS_1_;
	uint depth : DEPTH_1_;
	
	float4 color : COLOR_2_;
	float4 borderColor : BORDER_COLOR_2_;
	float borderWidth : BORDER_WIDTH_2_;
	float borderHardness : BORDER_HARDNESS_2_;
	
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float2 localPosition : LOCAL_POSITION;
	nointerpolation float4 bounds : BOUNDS;
	nointerpolation float4 color : COLOR;
	nointerpolation float4 borderColor : BORDER_COLOR;
	nointerpolation float borderWidth : BORDER_WIDTH;
	nointerpolation float borderHardness : BORDER_HARDNESS;
};

struct PS_Input : VS_Output
{
};












#section vertex

RootConstant<float> u_AspectRatio;
RootConstant<uint> u_MaxDepth;
struct Dummy {}; // only used to fix intelisence

VS_Output main(VS_Input input)
{

	VS_Output output;
	
	output.localPosition = (input.position * input.bounds.zw);
	output.position.xy = output.localPosition + input.bounds.xy;
	output.bounds = input.bounds;
	
	output.position.x /= u_AspectRatio;
	output.position.y = 1-output.position.y;
	output.position.xy = (output.position.xy * 2) - 1;

	output.position.z = (float)input.depth / (float)u_MaxDepth;
	output.position.w = 1;

	output.color = input.color;
	
	output.borderColor = input.borderColor;
	output.borderWidth = input.borderWidth;
	output.borderHardness = input.borderHardness;
	
	return output;
}







#section pixel

float harden(float val, float hardness)
{
	return pow(val, 1-hardness);
}


struct PS_Output
{
	float4 color : SV_TARGET0;
};

[earlydepthstencil]
PS_Output main(PS_Input input)
{
	float2 size = input.bounds.zw;
	
	PS_Output output;

	output.color = input.color;
	
	float distToEdge = FLT_MAX;
	distToEdge = min(distToEdge, abs(input.localPosition.x - 0));
	distToEdge = min(distToEdge, abs(input.localPosition.x - size.x));
	distToEdge = min(distToEdge, abs(input.localPosition.y - 0));
	distToEdge = min(distToEdge, abs(input.localPosition.y - size.y));
	
	float edgeHardness = harden(saturate(smoothstep(1, 0, distToEdge / input.borderWidth)), input.borderHardness);
	
	output.color *= lerp(output.color, input.borderColor, edgeHardness);
	
	return output;
}








