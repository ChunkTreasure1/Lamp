#pragma once
#include "Cameras/PerspectiveCamera.h"

namespace Lamp
{
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(PerspectiveCamera& camera);
		static void End();

		static void TestDraw();
	};
}