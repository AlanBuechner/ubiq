#include "pch.h"
#include "MeshBuilder.h"


namespace Engine
{

	MeshBuilder CreateUVSphere(uint32 uSeg, uint32 vSeg)
	{
		MeshBuilder builder;

		for (uint32 u = 0; u <= uSeg; u++)
		{
			float t1 = Math::PI * ((float)u / (float)uSeg);
			for (uint32 v = 0; v < vSeg; v++)
			{
				float t2 = Math::PI * ((float)v / (float)vSeg) * 2;
				RendererVertex vertex;
				vertex.position = Math::SphericalToCartesian({ t1 - Math::PI / 2, t2 });
				vertex.normal = vertex.position;
				vertex.uv = { u / uSeg, v / vSeg };
				builder.vertices.Push(vertex);
			}
		}

		for (uint32 u = 0; u < uSeg; u++)
		{
			for (uint32 v = 0; v < vSeg; v++)
			{
				uint32 a = v % vSeg * vSeg + u % uSeg;
				uint32 b = (v % vSeg + 1) * vSeg + u % uSeg;
				uint32 c = v % vSeg * vSeg + (u % uSeg + 1);
				uint32 d = (v % vSeg + 1) * vSeg + (u % uSeg + 1);

				builder.indices.Push(a);
				builder.indices.Push(b);
				builder.indices.Push(c);
				builder.indices.Push(b);
				builder.indices.Push(d);
				builder.indices.Push(c);
			}
		}

		return builder;
	}

}
