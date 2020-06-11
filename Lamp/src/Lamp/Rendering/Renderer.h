#pragma once
#include "Cameras/OrthographicCameraController.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Event/Event.h"
#include "Lamp/Core/Application.h"
#include <algorithm>

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Rendering/Vertices/VertexArray.h"

namespace Lamp
{
	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(OrthographicCamera& camera);
		static void End();

		static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray);
		static void DrawIndexedLines(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount);
		static void SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.g, color.a); }
		static void Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

		static void OnWindowResize(uint32_t width, uint32_t height);

	private: 
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_pSceneData;
	};
}