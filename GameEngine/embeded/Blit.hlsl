#section config

passes = {
	Blit = {
		CS = Blit;
	};
	BlurX = {
		CS = BlurX;
	};
	BlurY = {
		CS = BlurY;
	};
};






#section Blit

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

void SampleTexture(uint2 pixel, inout float4 color, uint2 texelSize)
{
	//if (pixel.x >= texelSize.x)
	//	pixel.x = texelSize.x - 1;
	//if (pixel.y >= texelSize.y)
	//	pixel.y = texelSize.y - 1;

	color += SrcTexture[pixel];
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 destTexelSize;
	DstTexture.GetDimensions(destTexelSize.x, destTexelSize.y);
	if (DTid.x < destTexelSize.x && DTid.y < destTexelSize.y)
	{
		uint2 texcoords = 2 * DTid.xy;

		float4 color = float4(0, 0, 0, 0);
		SampleTexture(texcoords + uint2(0, 0), color, destTexelSize);
		SampleTexture(texcoords + uint2(1, 0), color, destTexelSize);
		SampleTexture(texcoords + uint2(0, 1), color, destTexelSize);
		SampleTexture(texcoords + uint2(1, 1), color, destTexelSize);

		//Write the final color into the destination texture.
		DstTexture[DTid.xy] = color / 4.0;
	}
}




#section BlurX

static const uint cashSize = 64+2+2;
groupshared float4 gs_cash[cashSize];

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	[unroll]
	for(uint i = 0; i < cashSize; i++)
		gs_cash[i] = SrcTexture[int2(Gid.x*64, Gid.y) + int2(i-2, 0)];

	GroupMemoryBarrierWithGroupSync();

	float4 color = float4(0, 0, 0, 0);
	for(i = 0; i < 5; i++)
		color += gs_cash[GTid.x + i];

	DstTexture[DTid.xy] = color / 5.0;

}


#section BlurY

static const uint cashSize = 64+2+2;
groupshared float4 gs_cash[cashSize];

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;

[numthreads(1, 64, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	[unroll]
	for(uint i = 0; i < cashSize; i++)
		gs_cash[i] = SrcTexture[int2(Gid.x, Gid.y*64) + int2(0, i-2)];

	GroupMemoryBarrierWithGroupSync();

	float4 color = float4(0, 0, 0, 0);
	for(i = 0; i < 5; i++)
		color += gs_cash[GTid.y + i];

	DstTexture[DTid.xy] = color / 5.0;

}
