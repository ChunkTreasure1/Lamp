#pragma once
#include <glm/glm.hpp>

#include "CameraBase.h"

namespace Lamp
{
	class OrthographicCamera : public CameraBase
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		//Setting
		void SetProjection(float left, float right, float bottom, float top) override;
		Frustum CreateFrustum() override { return Frustum(); }

	protected:
		void RecalculateViewMatrix() override;
	};
}