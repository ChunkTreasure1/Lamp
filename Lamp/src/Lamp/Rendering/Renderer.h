#pragma once
#include "Cameras/OrthographicCameraController.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Core/Application.h"
#include <algorithm>

#include "RendererAPI.h"

namespace Lamp
{
	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(OrthographicCamera& camera);
		static void End();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private: 
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_pSceneData;
	};
}