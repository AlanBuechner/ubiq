#section config

passes = {
	Blit = {
		CS = Blit;
	};
};




#section Blit

RWTexture2D<float4> SrcTexture;
RWTexture2D<float4> DstTexture;
sampler BilinearClamp;
StaticSampler textureSampler = StaticSampler(clamp, clamp, linear, linear);

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