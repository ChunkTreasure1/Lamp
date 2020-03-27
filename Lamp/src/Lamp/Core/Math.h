#pragma once

#include <math.h>

namespace Lamp
{
	class Math
	{
	public:
		static bool SolveQuadric(const float& a, const float& b, const float& c, float& x0, float& x1)
		{
			float discr = b * b - 4 * a * c;
			if (discr < 0)
			{
				return false;
			}
			else if (discr == 0)
			{
				x0 = x1 = -0.5f * b / a;
			}
			else 
			{
				float q = (b > 0) ?
					-0.5f * (b + sqrt(discr)) :
					-0.5f * (b - sqrt(discr));

				x0 = q / a;
				x1 = c / q;
			}

			return true;
		}

	private:
		Math() = delete;
	};
}