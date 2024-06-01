#include "pch.h"
#include "Light.h"

namespace Game
{

	Math::Vector3 CCTToRGB(float k)
	{
		Math::Vector3 color;
		float temp = k / 100;

		if (temp <= 66)
			color.r = 255;
		else
		{
			color.r = temp - 60;
			color.r = 329.698727446 * Math::Pow(color.r, -0.1332047592);
			if (color.r < 0) color.r = 0;
			if (color.r > 255) color.r = 255;
		}
		color.r /= 255;

		if (temp <= 66)
		{
			color.g = temp;
			color.g = 99.4708025861 * Math::Log(color.g) - 161.1195681661;
		}
		else
		{
			color.g = temp - 60;
			color.g = 288.1221695283 * Math::Pow(color.g, -0.0755148492);
		}
		if (color.g < 0) color.g = 0;
		if (color.g > 255) color.g = 255;
		color.g /= 255;

		if (temp >= 66)
			color.b = 255;
		else
		{
			if (temp <= 19)
				color.b = 0;
			else
			{
				color.b = temp - 10;
				color.b = 138.5177312231 * log(color.b) - 305.0447927307;
			}
		}
		if (color.b < 0) color.b = 0;
		if (color.b > 255) color.b = 255;
		color.b /= 255;

		return color;
	}

}

