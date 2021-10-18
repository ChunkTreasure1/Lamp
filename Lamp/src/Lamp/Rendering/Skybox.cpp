#include "lppch.h"
#include "Skybox.h"

#include "Vertices/VertexBuffer.h"
#include "Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Texture2D/IBLBuffer.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		std::vector<float> boxPositions =
		{
			-1, -1, -1,
			 1, -1, -1,
			 1,  1, -1,
			-1,  1, -1,
			-1, -1,  1,
			 1, -1,  1,
			 1,  1,  1,
			-1,  1,  1
		};

		std::vector<uint32_t> boxIndicies =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		m_vertexArray = VertexArray::Create();
		Ref<VertexBuffer> buffer = VertexBuffer::Create(boxPositions, (uint32_t)(sizeof(float) * boxPositions.size()));
		buffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" }
		});

		m_vertexArray->AddVertexBuffer(buffer);

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(boxIndicies, (uint32_t)(boxIndicies.size()));
		m_vertexArray->SetIndexBuffer(indexBuffer);

		m_shader = ShaderLibrary::GetShader("Skybox");

		m_iblBuffer = CreateRef<IBLBuffer>(path.string());
		Renderer::s_pSceneData->internalFramebuffers.emplace(std::make_pair("Skybox", m_iblBuffer));
	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Render()
	{
		m_shader->Bind();
		m_shader->UploadInt("u_EnvironmentMap", 0);
		m_iblBuffer->Bind();

		Renderer3D::DrawCube();
	}
}