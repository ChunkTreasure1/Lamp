#pragma once

#include <glm/glm.hpp>
#include "Texture2D/Texture2D.h"
#include "OrthographicCamera.h"

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(const OrthographicCamera camera);
		static void End();
	
		static void DrawModel(const glm::vec3& pos, const glm::vec3 scale);
	};
}