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
		/////Skybox//////
		Ref<Shader> SkyboxShader;
		Ref<VertexArray> SkyboxVertexArray;
		/////////////////

		/////Quad/////	
		Ref<VertexArray> QuadVertexArray;
		//////////////

		RenderPassSpecification* CurrentRenderPass = nullptr;

	};

	static Renderer3DStorage* s_pRenderData;
	RenderBuffer Renderer3D::s_RenderBuffer;

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
	}

	void Renderer3D::BeginPass(RenderPassSpecification& passSpec)
	{
		s_pRenderData->CurrentRenderPass = &passSpec;
	}

	void Renderer3D::EndPass()
	{
		s_pRenderData->CurrentRenderPass = nullptr;
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, const Ref<VertexArray> vertexData, const Ref<Material> material, size_t objectId)
	{
		LP_ASSERT(s_pRenderData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();

		const auto& pass = s_pRenderData->CurrentRenderPass;

		RenderCommand::SetCullFace(pass->cullFace);

		Ref<Shader> shaderToUse = pass->renderShader ? pass->renderShader : material->GetShader();

		shaderToUse->Bind();

		{
			LP_PROFILE_SCOPE("StaticUniorms");
			//Static Uniforms
			for (const auto& staticUniformPair : pass->staticUniforms)
			{
				const auto& staticUniformSpec = staticUniformPair.second;

				if (staticUniformSpec.data.type() == typeid(RenderData))
				{
					RenderData type = std::any_cast<RenderData>(staticUniformSpec.data);
					switch (type)
					{
						case Lamp::RenderData::Transform:
							shaderToUse->UploadMat4(staticUniformSpec.name, modelMatrix);
							break;
						case Lamp::RenderData::ID:
							shaderToUse->UploadInt(staticUniformSpec.name, objectId);
							break;
					}

					continue;
				}

				switch (staticUniformSpec.type)
				{
					case UniformType::Int:
						shaderToUse->UploadInt(staticUniformSpec.name, std::any_cast<int>(staticUniformSpec.data));
						break;

					case UniformType::Float:
						shaderToUse->UploadFloat(staticUniformSpec.name, std::any_cast<float>(staticUniformSpec.data));
						break;

					case UniformType::Float2:
						shaderToUse->UploadFloat2(staticUniformSpec.name, std::any_cast<glm::vec2>(staticUniformSpec.data));
						break;

					case UniformType::Float3:
						shaderToUse->UploadFloat3(staticUniformSpec.name, std::any_cast<glm::vec3>(staticUniformSpec.data));
						break;

					case UniformType::Float4:
						shaderToUse->UploadFloat4(staticUniformSpec.name, std::any_cast<glm::vec4>(staticUniformSpec.data));
						break;

					case UniformType::Mat4:
						shaderToUse->UploadMat4(staticUniformSpec.name, std::any_cast<glm::mat4>(staticUniformSpec.data));
						break;
				}
			}
		}
		{
			LP_PROFILE_SCOPE("DynamicUniorms");
			//Dynamic Uniforms
			for (const auto& dynamicUniformPair : pass->dynamicUniforms)
			{
				const auto& dynamicUniformSpec = dynamicUniformPair.second.first;

				if (dynamicUniformSpec.data == nullptr)
				{
					LP_CORE_ERROR("Dynamic uniform data is nullptr at {0}", dynamicUniformSpec.name);
					continue;
				}

				switch (dynamicUniformSpec.type)
				{
					case UniformType::Int:
						shaderToUse->UploadInt(dynamicUniformSpec.name, *static_cast<int*>(dynamicUniformSpec.data));
						break;

					case UniformType::Float:
						shaderToUse->UploadFloat(dynamicUniformSpec.name, *static_cast<float*>(dynamicUniformSpec.data));
						break;

					case UniformType::Float3:
						shaderToUse->UploadFloat3(dynamicUniformSpec.name, *static_cast<glm::vec3*>(dynamicUniformSpec.data));
						break;

					case UniformType::Float4:
						shaderToUse->UploadFloat4(dynamicUniformSpec.name, *static_cast<glm::vec4*>(dynamicUniformSpec.data));
						break;

					case UniformType::Mat4:
						shaderToUse->UploadMat4(dynamicUniformSpec.name, *static_cast<glm::mat4*>(dynamicUniformSpec.data));
						break;
				}
			}
		}
		{
			LP_PROFILE_SCOPE("Framebuffers");
			//Framebuffers
			for (const auto& framebufferPair : pass->framebuffers)
			{
				const auto& spec = framebufferPair.second.first;

				if (spec.framebuffer == nullptr)
				{
					LP_CORE_ERROR("Framebuffer is nullptr at {0}!", spec.name);
					continue;
				}

				for (const auto& attachment : spec.attachments)
				{
					switch (attachment.type)
					{
						case TextureType::Color:
							spec.framebuffer->BindColorAttachment(attachment.bindId, attachment.attachmentId);
							break;

						case TextureType::Depth:
							spec.framebuffer->BindDepthAttachment(attachment.bindId);
							break;

						default:
							break;
					}
				}
			}
		}
		{
			LP_PROFILE_SCOPE("Textures");
			int i = 0;
			for (const auto& textureName : material->GetShader()->GetSpecifications().TextureNames)
			{
				material->GetTextures()[textureName]->Bind(i);
				i++;
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
		}
	}

	void Renderer3D::DrawCube()
	{
		s_pRenderData->SkyboxVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pRenderData->SkyboxVertexArray, s_pRenderData->SkyboxVertexArray->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::DrawQuad()
	{
		s_pRenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pRenderData->QuadVertexArray, s_pRenderData->QuadVertexArray->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::RenderQuad()
	{
		const auto& pass = s_pRenderData->CurrentRenderPass;
		RenderCommand::SetCullFace(pass->cullFace);
		Ref<Shader> shaderToUse = pass->renderShader;

		shaderToUse->Bind();

		//Static Uniforms
		for (const auto& staticUniformPair : pass->staticUniforms)
		{
			const auto& staticUniformSpec = staticUniformPair.second;

			switch (staticUniformSpec.type)
			{
				case UniformType::Int:
					shaderToUse->UploadInt(staticUniformSpec.name, std::any_cast<int>(staticUniformSpec.data));
					break;

				case UniformType::Float:
					shaderToUse->UploadFloat(staticUniformSpec.name, std::any_cast<float>(staticUniformSpec.data));
					break;

				case UniformType::Float2:
					shaderToUse->UploadFloat2(staticUniformSpec.name, std::any_cast<glm::vec2>(staticUniformSpec.data));
					break;

				case UniformType::Float3:
					shaderToUse->UploadFloat3(staticUniformSpec.name, std::any_cast<glm::vec3>(staticUniformSpec.data));
					break;

				case UniformType::Float4:
					shaderToUse->UploadFloat4(staticUniformSpec.name, std::any_cast<glm::vec4>(staticUniformSpec.data));
					break;

				case UniformType::Mat4:
					shaderToUse->UploadMat4(staticUniformSpec.name, std::any_cast<glm::mat4>(staticUniformSpec.data));
					break;
			}
		}

		//Dynamic Uniforms
		for (const auto& dynamicUniformPair : pass->dynamicUniforms)
		{
			const auto& dynamicUniformSpec = dynamicUniformPair.second.first;
			switch (dynamicUniformSpec.type)
			{
				case UniformType::Int:
					shaderToUse->UploadInt(dynamicUniformSpec.name, *static_cast<int*>(dynamicUniformSpec.data));
					break;

				case UniformType::Float:
					shaderToUse->UploadFloat(dynamicUniformSpec.name, *static_cast<float*>(dynamicUniformSpec.data));
					break;

				case UniformType::Float3:
					shaderToUse->UploadFloat3(dynamicUniformSpec.name, *static_cast<glm::vec3*>(dynamicUniformSpec.data));
					break;

				case UniformType::Float4:
					shaderToUse->UploadFloat4(dynamicUniformSpec.name, *static_cast<glm::vec4*>(dynamicUniformSpec.data));
					break;

				case UniformType::Mat4:
					shaderToUse->UploadMat4(dynamicUniformSpec.name, *static_cast<glm::mat4*>(dynamicUniformSpec.data));
					break;
			}
		}

		//Framebuffers
		for (const auto& framebufferPair : pass->framebuffers)
		{
			const auto& spec = framebufferPair.second.first;

			if (spec.framebuffer == nullptr)
			{
				LP_CORE_ERROR("Framebuffer is nullptr at {0}!", spec.name);
				continue;
			}

			for (const auto& attachment : spec.attachments)
			{
				switch (attachment.type)
				{
					case TextureType::Color:
						spec.framebuffer->BindColorAttachment(attachment.bindId, attachment.attachmentId);
						break;

					case TextureType::Depth:
						spec.framebuffer->BindDepthAttachment(attachment.bindId);
						break;

					default:
						break;
				}
			}
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

			s_pRenderData->QuadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(quadVertices, (uint32_t)(sizeof(Vertex) * quadVertices.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
				});
			s_pRenderData->QuadVertexArray->AddVertexBuffer(pBuffer);
			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(quadIndices, (uint32_t)quadIndices.size());
			s_pRenderData->QuadVertexArray->SetIndexBuffer(indexBuffer);
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

			s_pRenderData->SkyboxVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(boxPositions, (uint32_t)(sizeof(float) * boxPositions.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" }
				});
			s_pRenderData->SkyboxVertexArray->AddVertexBuffer(pBuffer);

			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(boxIndicies, (uint32_t)(boxIndicies.size()));
			s_pRenderData->SkyboxVertexArray->SetIndexBuffer(indexBuffer);
			s_pRenderData->SkyboxShader = ShaderLibrary::GetShader("Skybox");

			Renderer::s_pSceneData->internalFramebuffers.emplace(std::make_pair("Skybox", CreateRef<IBLBuffer>("assets/textures/Frozen_Waterfall_Ref.hdr")));
		}
		////////////////
	}
}