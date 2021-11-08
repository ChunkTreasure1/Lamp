#include "lppch.h"
#include "Renderer3D.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Texture2D/IBLBuffer.h"
#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Level/Level.h"
#include "Texture2D/IBLBuffer.h"

#include "Buffers/UniformBuffer.h"
#include "Buffers/ShaderStorageBuffer.h"
#include "UniformBuffers.h"
#include "LightBase.h"

#include <random>
#include <glad/glad.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	struct Renderer3DStorage
	{
		Ref<VertexArray> cubeVertexArray;
		Ref<VertexArray> quadVertexArray;
		Ref<CameraBase> camera;

		const RenderPassSpecification* currentRenderPass = nullptr;
	};

	static Renderer3DStorage* s_pRenderData;
	RenderBuffer Renderer3D::s_RenderBuffer;
	Renderer3D::Statistics Renderer3D::s_renderStatistics;

	void Renderer3D::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pRenderData = new Renderer3DStorage();

		CreateBaseMeshes();

		s_RenderBuffer.drawCalls.reserve(1000);
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pRenderData;
	}

	void Renderer3D::Begin(const Ref<CameraBase> camera)
	{
		s_pRenderData->camera = camera;
		s_renderStatistics.totalDrawCalls = 0;
		s_renderStatistics.otherDrawCalls = 0;
		s_renderStatistics.sceneDrawCalls = 0;

		LP_PROFILE_FUNCTION();

		//Draw shadow maps 
		for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
		{
			if (!light->castShadows)
			{
				continue;
			}
			light->shadowBuffer->Bind();
			RenderCommand::ClearDepth();
			BeginPass(light->shadowPass->GetSpecification());

			DrawRenderBuffer();

			EndPass();
			light->shadowBuffer->Unbind();
		}
	}

	void Renderer3D::End()
	{
		LP_PROFILE_FUNCTION();
		s_RenderBuffer.drawCalls.clear();
		s_renderStatistics.totalDrawCalls = s_renderStatistics.sceneDrawCalls + s_renderStatistics.otherDrawCalls;
	}

	void Renderer3D::BeginPass(const RenderPassSpecification& passSpec)
	{
		s_pRenderData->currentRenderPass = &passSpec;
	}

	void Renderer3D::EndPass()
	{
		s_pRenderData->currentRenderPass = nullptr;
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, const Ref<VertexArray> vertexData, const Ref<Material> material, size_t objectId)
	{
		LP_ASSERT(s_pRenderData->currentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();

		const auto& pass = s_pRenderData->currentRenderPass;

		RenderCommand::SetCullFace(pass->cullFace);

		Ref<Shader> shaderToUse = pass->renderShader ? pass->renderShader : material->GetShader();

		shaderToUse->Bind();

		{
			LP_PROFILE_SCOPE("Uniforms");
			for (const auto& uniformSpec : pass->uniforms)
			{
				if (uniformSpec.data.type() == typeid(RenderData))
				{
					RenderData type = std::any_cast<RenderData>(uniformSpec.data);
					switch (type)
					{
						case Lamp::RenderData::Transform:
							shaderToUse->UploadMat4(uniformSpec.name, modelMatrix);
							break;
						case Lamp::RenderData::ID:
							shaderToUse->UploadInt(uniformSpec.name, objectId);
							break;
					}

					continue;
				}

				if (uniformSpec.pData == nullptr && !uniformSpec.data.has_value())
				{
					LP_CORE_ERROR("No data at {0}!", uniformSpec.name);
					continue;
				}

				switch (uniformSpec.type)
				{
					case UniformType::Int: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
					case UniformType::Float: shaderToUse->UploadFloat(uniformSpec.name, uniformSpec.pData ? *static_cast<float*>(uniformSpec.pData) : std::any_cast<float>(uniformSpec.data)); break;
					case UniformType::Float2: shaderToUse->UploadFloat2(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec2*>(uniformSpec.pData) : std::any_cast<glm::vec2>(uniformSpec.data)); break;
					case UniformType::Float3: shaderToUse->UploadFloat3(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec3*>(uniformSpec.pData) : std::any_cast<glm::vec3>(uniformSpec.data)); break;
					case UniformType::Float4: shaderToUse->UploadFloat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec4*>(uniformSpec.pData) : std::any_cast<glm::vec4>(uniformSpec.data)); break;
					case UniformType::Mat3: shaderToUse->UploadMat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::mat3*>(uniformSpec.pData) : std::any_cast<glm::mat3>(uniformSpec.data)); break;
					case UniformType::Mat4: shaderToUse->UploadMat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::mat4*>(uniformSpec.pData) : std::any_cast<glm::mat4>(uniformSpec.data)); break;
					case UniformType::Sampler2D: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
					case UniformType::SamplerCube: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
				}
			}
		}
		{
			LP_PROFILE_SCOPE("Framebuffers");
			//Framebuffers
			for (const auto& spec : pass->framebuffers)
			{

				if (!spec.framebuffer)
				{
					LP_CORE_ERROR("Framebuffer is nullptr at {0}!", spec.name);
					continue;
				}

				for (const auto& attachment : spec.attachments)
				{
					switch (attachment.type)
					{
						case TextureType::Color: spec.framebuffer->BindColorAttachment(attachment.bindId, attachment.attachmentId); break;
						case TextureType::Depth: spec.framebuffer->BindDepthAttachment(attachment.bindId); break;
					}
				}
			}
		}
		{
			LP_PROFILE_SCOPE("Textures");

			int i = 0;
			for (const auto& textureName : material->GetShader()->GetSpecification().textureNames)
			{
				material->GetTextures()[textureName]->Bind(i);
				i++;
			}

			//Textures
			for (const auto& textureSpec : pass->textures)
			{
				if (!textureSpec.texture)
				{
					LP_CORE_ERROR("Texture is nullptr");
					continue;
				}

				textureSpec.texture->Bind(textureSpec.bindSlot);
			}

			/////Testing/////
			uint32_t index = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				if (!light->castShadows)
				{
					return;
				}

				shaderToUse->UploadInt("u_DirShadowMaps[" + std::to_string(index) + "]", i);
				light->shadowBuffer->BindDepthAttachment(i);
				i++;
				index++;
			}
		}

		{
			vertexData->Bind();
			RenderCommand::DrawIndexed(vertexData, vertexData->GetIndexBuffer()->GetCount());
			s_renderStatistics.sceneDrawCalls++;
		}
	}

	void Renderer3D::DrawCube()
	{
		s_pRenderData->cubeVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pRenderData->cubeVertexArray, s_pRenderData->cubeVertexArray->GetIndexBuffer()->GetCount());
		s_renderStatistics.otherDrawCalls++;
	}

	void Renderer3D::DrawQuad()
	{
		s_pRenderData->quadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pRenderData->quadVertexArray, s_pRenderData->quadVertexArray->GetIndexBuffer()->GetCount());
		s_renderStatistics.otherDrawCalls++;
	}

	void Renderer3D::RenderQuad()
	{
		const auto& pass = s_pRenderData->currentRenderPass;
		RenderCommand::SetCullFace(pass->cullFace);
		Ref<Shader> shaderToUse = pass->renderShader;

		shaderToUse->Bind();

		{
			LP_PROFILE_SCOPE("Uniforms");
			for (const auto& uniformSpec : pass->uniforms)
			{
				if (!uniformSpec.pData && !uniformSpec.data.has_value())
				{
					LP_CORE_ERROR("No data at {0}!", uniformSpec.name);
					continue;
				}

				switch (uniformSpec.type)
				{
					case UniformType::Int: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
					case UniformType::Float: shaderToUse->UploadFloat(uniformSpec.name, uniformSpec.pData ? *static_cast<float*>(uniformSpec.pData) : std::any_cast<float>(uniformSpec.data)); break;
					case UniformType::Float2: shaderToUse->UploadFloat2(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec2*>(uniformSpec.pData) : std::any_cast<glm::vec2>(uniformSpec.data)); break;
					case UniformType::Float3: shaderToUse->UploadFloat3(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec3*>(uniformSpec.pData) : std::any_cast<glm::vec3>(uniformSpec.data)); break;
					case UniformType::Float4: shaderToUse->UploadFloat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::vec4*>(uniformSpec.pData) : std::any_cast<glm::vec4>(uniformSpec.data)); break;
					case UniformType::Mat3: shaderToUse->UploadMat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::mat3*>(uniformSpec.pData) : std::any_cast<glm::mat3>(uniformSpec.data)); break;
					case UniformType::Mat4: shaderToUse->UploadMat4(uniformSpec.name, uniformSpec.pData ? *static_cast<glm::mat4*>(uniformSpec.pData) : std::any_cast<glm::mat4>(uniformSpec.data)); break;
					case UniformType::Sampler2D: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
					case UniformType::SamplerCube: shaderToUse->UploadInt(uniformSpec.name, uniformSpec.pData ? *static_cast<int*>(uniformSpec.pData) : std::any_cast<int>(uniformSpec.data)); break;
				}
			}
		}

		//Framebuffers
		for (const auto& spec : pass->framebuffers)
		{
			if (!spec.framebuffer)
			{
				LP_CORE_ERROR("Framebuffer is nullptr at {0}!", spec.name);
				continue;
			}

			for (const auto& attachment : spec.attachments)
			{
				switch (attachment.type)
				{
					case TextureType::Color: spec.framebuffer->BindColorAttachment(attachment.bindId, attachment.attachmentId); break;
					case TextureType::Depth: spec.framebuffer->BindDepthAttachment(attachment.bindId); break;
				}
			}
		}

		//Textures
		for (const auto& textureSpec : pass->textures)
		{
			if (!textureSpec.texture)
			{
				LP_CORE_ERROR("Texture is nullptr");
				continue;
			}

			textureSpec.texture->Bind(textureSpec.bindSlot);
		}

		DrawQuad();
	}

	void Renderer3D::SetEnvironment(const std::string& path)
	{
		Renderer::s_pSceneData->internalFramebuffers["Skybox"] = CreateRef<IBLBuffer>(path);
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = mesh->GetVertexArray();

		s_RenderBuffer.drawCalls.push_back(data);
	}

	void Renderer3D::DrawRenderBuffer()
	{
		LP_PROFILE_FUNCTION();

		//TODO: Should be moved to only be made once, right after render event
		//Sort
		const glm::vec3& pos = s_pRenderData->camera->GetPosition();
		std::sort(s_RenderBuffer.drawCalls.begin(), s_RenderBuffer.drawCalls.end(), [&pos](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distOne = glm::exp2(pos.x - dPosOne.x) + glm::exp2(pos.y - dPosOne.y) + glm::exp2(pos.z - dPosOne.z);
				const float distTwo = glm::exp2(pos.x - dPosTwo.x) + glm::exp2(pos.y - dPosTwo.y) + glm::exp2(pos.z - dPosTwo.z);

				return distOne < distTwo;
			});

		for (const auto& data : s_RenderBuffer.drawCalls)
		{
			DrawMesh(data.transform, data.data, data.material, data.id);
		}
	}

	void Renderer3D::CreateBaseMeshes()
	{
		/////Quad/////
		{
			std::vector<Vertex> quadVertices =
			{
				Vertex({ 1.f, 1.f, 0.f }, { 1.f, 1.f }),
				Vertex({ 1.f, -1.f, 0.f }, { 1.f, 0.f }),
				Vertex({ -1.f, -1.f, 0.f }, { 0.f, 0.f }),
				Vertex({ -1.f, 1.f, 0.f }, { 0.f, 1.f }),
			};

			std::vector<uint32_t> quadIndices =
			{
				0, 3, 1, //(top right - bottom right - top left)
				1, 3, 2 //(bottom right - bottom left - top left)
			};

			s_pRenderData->quadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(quadVertices, (uint32_t)(sizeof(Vertex) * quadVertices.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
				});
			s_pRenderData->quadVertexArray->AddVertexBuffer(pBuffer);
			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(quadIndices, (uint32_t)quadIndices.size());
			s_pRenderData->quadVertexArray->SetIndexBuffer(indexBuffer);
		}
		//////////////

		/////Skybox/////
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

			s_pRenderData->cubeVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(boxPositions, (uint32_t)(sizeof(float) * boxPositions.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" }
				});
			s_pRenderData->cubeVertexArray->AddVertexBuffer(pBuffer);

			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(boxIndicies, (uint32_t)(boxIndicies.size()));
			s_pRenderData->cubeVertexArray->SetIndexBuffer(indexBuffer);
		}
		////////////////
	}
}