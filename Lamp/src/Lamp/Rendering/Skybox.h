#pragma once

#include "Lamp/Rendering/Vertices/VertexArray.h"

#include <filesystem>

namespace Lamp
{
	class Shader;
	class IBLBuffer;
	class Skybox
	{
	public:
		Skybox(const std::filesystem::path& path);
		~Skybox();

		void Render();

	public:
		static Ref<Skybox> Create(const std::filesystem::path& path) { return CreateRef<Skybox>(path); }

	private:
		Ref<VertexArray> m_vertexArray;
		Ref<IBLBuffer> m_iblBuffer;
		Ref<Shader> m_shader;
	};
}