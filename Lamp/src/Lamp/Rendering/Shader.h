#pragma once

#include <string>

namespace Lamp
{
	class Shader
	{
	public:
		Shader(std::string& fragment, std::string& vertex);
		~Shader();

		void Bind();
		void Unbind();

	private:
		uint32_t m_RendererID;
	};
}