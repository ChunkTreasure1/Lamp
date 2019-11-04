#pragma once
#include "OrthographicCameraController.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Event/Event.h"
#include "Lamp/Core/Application.h"
#include <algorithm>

#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Rendering/Vertecies/VertexArray.h"

namespace Lamp
{
	class Renderer
	{
	public:
		static void Initialize();

		static void Begin(OrthographicCamera& camera);
		static void End();

		static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray);
		static void SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.g, color.a); glClearDepth(1.f); }
		static void Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

	private: 
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_pSceneData;
	};
}