#pragma once

#include <random>

namespace Lamp
{
	static std::random_device tempDev;

	class Random
	{
	public:
		static int Int(int aMin, int aMax)
		{
			std::mt19937 tempRand(tempDev());
			std::uniform_int_distribution<std::mt19937::result_type> tempDist(aMin, aMax);

			return tempDist(tempRand);
		}

		static float Float(float aMin, float aMax)
		{
			std::mt19937 rand(tempDev());
			std::uniform_real_distribution<float> dist(aMin, aMax);

			return dist(rand);
		}
	private:
		Random() = delete;
	};
}