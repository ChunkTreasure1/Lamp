#pragma once

#include "Lamp/Math/Math.h"

namespace Lamp
{
	template<typename T>
	class BezierSpline
	{
	public:
		T p1; // Point 1
		T p2; // Point 2
		T c1; // Control 1
		T c2; // Control 2

		static T Interpolate(BezierSpline<T>& curve, float t);

	private:
	};

	template<typename T>
	inline T BezierSpline<T>::Interpolate(BezierSpline<T>& curve, float t)
	{
		T result = curve.p1 * ((1.f - t) * (1.f - t) * (1.f - t)) +
			curve.c1 * (3.f * ((1.f - t) * (1.f - t)) * t) +
			curve.c2 * (3.f * (1.f - t) * (t * t)) +
			curve.p2 * (t * t * t);

		return result;
	}

}