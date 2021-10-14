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
#include "RenderGraph/Nodes/DynamicUniformRegistry.h"

#include "Buffers/UniformBuffer.h"
#include "Buffers/ShaderStorageBuffer.h"
#include "UniformBuffers.h"

#include <random>
#include <glad/glad.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer3DStorage
	{
		static const uint32_t MaxLights = 1024;

		/////Skybox//////
		Ref<Shader> SkyboxShader;
		Ref<VertexArray> SkyboxVertexArray;
		/////////////////

		/////Quad/////	
		Ref<VertexArray> QuadVertexArray;
		//////////////

		Ref<CameraBase> Camera;

		RenderPassSpecification* CurrentRenderPass = nullptr;

		/////Shadows/////
		Ref<Framebuffer> ShadowBuffer;
		/////////////////

		/////SSAO/////
		std::vector<glm::vec3> SSAONoise;
		std::vector<glm::vec3> SSAOKernel;
		//////////////

		glm::vec2 LastViewportSize = glm::vec2(0.f);

		/////Uniform buffers/////
		CommonBuffer CommonBuffer;
		Ref<UniformBuffer> CommonUniformBuffer;

		DirectionalLightBuffer DirectionalLightDataBuffer;
		Ref<UniformBuffer> DirectionalLightUniformBuffer;
		////////////////////////

		/////Forward plus/////
		uint32_t ForwardTileCount = 1;

		Ref<ShaderStorageBuffer> VisibleLightsBuffer;
		Ref<ShaderStorageBuffer> PointLightStorageBuffer;
		//////////////////////
	};

	static Renderer3DStorage* s_pRenderData;
	RendererSettings* Renderer3D::s_RendererSettings;
	RenderBuffer Renderer3D::s_RenderBuffer;

	static float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pRenderData = new Renderer3DStorage();
		s_RendererSettings = new RendererSettings();

		CreateBaseMeshes();

		/////Uniform Buffer/////
		s_pRenderData->CommonUniformBuffer = UniformBuffer::Create(sizeof(CommonBuffer), 0);
		s_pRenderData->DirectionalLightUniformBuffer = UniformBuffer::Create(sizeof(DirectionalLightBuffer), 1);
		////////////////////////

		/////Storage buffer/////
		s_pRenderData->PointLightStorageBuffer = ShaderStorageBuffer::Create(s_pRenderData->MaxLights * sizeof(PointLightData), 2);

		s_RendererSettings->ForwardGroupX = (2000 + (2000 % 16)) / 16;
		s_RendererSettings->ForwardGroupY = (2000 + (2000 % 16)) / 16;

		s_pRenderData->ForwardTileCount = s_RendererSettings->ForwardGroupX * s_RendererSettings->ForwardGroupY;

		s_pRenderData->VisibleLightsBuffer = ShaderStorageBuffer::Create(s_pRenderData->ForwardTileCount * sizeof(LightIndex) * s_pRenderData->MaxLights, 3, DrawAccess::Static);
		////////////////////////

		//Setup dynamic uniforms
		DynamicUniformRegistry::AddUniform("Exposure", UniformType::Float, RegisterData(&Renderer3D::GetSettings().HDRExposure));
		DynamicUniformRegistry::AddUniform("Gamma", UniformType::Float, RegisterData(&Renderer3D::GetSettings().Gamma));
		DynamicUniformRegistry::AddUniform("Buffer Size", UniformType::Float2, RegisterData(&Renderer3D::GetSettings().BufferSize));
		DynamicUniformRegistry::AddUniform("ForwardTileX", UniformType::Int, RegisterData(&Renderer3D::GetSettings().ForwardGroupX));
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pRenderData;
		delete s_RendererSettings;
	}

	void Renderer3D::Begin(const Ref<CameraBase> camera)
	{
		SetupUniformBuffers(camera);
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

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Ref<VertexArray>& vertexData, Ref<Material> material, size_t objectId)
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
		}

		{
			vertexData->Bind();
			RenderCommand::DrawIndexed(vertexData, vertexData->GetIndexBuffer()->GetCount());
		}
	}

	void Renderer3D::DrawSkybox()
	{
		LP_ASSERT(s_pRenderData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		RenderCommand::SetCullFace(CullFace::Back);

		LP_PROFILE_FUNCTION();
		s_pRenderData->SkyboxShader->Bind();
		s_pRenderData->SkyboxShader->UploadInt("u_EnvironmentMap", 0);

		s_RendererSettings->InternalFramebuffers["Skybox"]->Bind();

		DrawCube();
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
		Ref<Shader> shaderToUse = pass->renderShader; //? pass->renderShader : mat.GetShader();

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
		s_RendererSettings->InternalFramebuffers["Skybox"] = CreateRef<IBLBuffer>(path);
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh>& mesh, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = mesh->GetVertexArray();

		s_RenderBuffer.drawCalls.push_back(data);
	}

	void Renderer3D::SubmitQuad(const glm::mat4& transform, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();
		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = s_pRenderData->QuadVertexArray;

		s_RenderBuffer.drawCalls.push_back(data);
	}

	void Renderer3D::DrawRenderBuffer()
	{
		LP_PROFILE_FUNCTION();

		for (auto& data : s_RenderBuffer.drawCalls)
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

			s_RendererSettings->InternalFramebuffers.emplace(std::make_pair("Skybox", CreateRef<IBLBuffer>("assets/textures/Frozen_Waterfall_Ref.hdr")));
		}
		////////////////
	}

	void Renderer3D::SetupUniformBuffers(const Ref<CameraBase> camera)
	{
		//Main
		{
			s_pRenderData->CommonBuffer.CameraPosition = glm::vec4(camera->GetPosition(), 0.f);
			s_pRenderData->CommonBuffer.Projection = camera->GetProjectionMatrix();
			s_pRenderData->CommonBuffer.View = camera->GetViewMatrix();
			s_pRenderData->CommonUniformBuffer->SetData(&s_pRenderData->CommonBuffer, sizeof(CommonBuffer));
		}

		//Directional light
		{
			uint32_t index = 0;
			s_pRenderData->DirectionalLightDataBuffer.lightCount = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				s_pRenderData->DirectionalLightDataBuffer.dirLights[index].direction = glm::vec4(glm::normalize(light->Direction), 0.f);
				s_pRenderData->DirectionalLightDataBuffer.dirLights[index].colorIntensity = glm::vec4(light->Color, light->Intensity);

				index++;
				s_pRenderData->DirectionalLightDataBuffer.lightCount++;
			}
			s_pRenderData->DirectionalLightUniformBuffer->SetData(&s_pRenderData->DirectionalLightDataBuffer, sizeof(DirectionalLightBuffer));
		}

		//Point light
		{
			PointLightData* buffer = (PointLightData*)s_pRenderData->PointLightStorageBuffer->Map();
			s_RendererSettings->LightCount = 0;

			for (uint32_t i = 0; i < g_pEnv->pLevel->GetRenderUtils().GetPointLights().size(); i++)
			{
				const auto& light = g_pEnv->pLevel->GetRenderUtils().GetPointLights()[i];

				buffer[i].position = glm::vec4(light->ShadowBuffer->GetPosition(), 0.f);
				buffer[i].color = glm::vec4(light->Color, 0.f);
				buffer[i].intensity = light->Intensity;
				buffer[i].falloff = light->Falloff;
				buffer[i].farPlane = light->FarPlane;
				buffer[i].radius = light->Radius;

				s_RendererSettings->LightCount++;
			}

			s_pRenderData->PointLightStorageBuffer->Unmap();
		}

		s_pRenderData->Camera = camera;

		s_RendererSettings->ForwardGroupX = ((uint32_t)s_RendererSettings->BufferSize.x + ((uint32_t)s_RendererSettings->BufferSize.x % 16)) / 16;
		s_RendererSettings->ForwardGroupY = ((uint32_t)s_RendererSettings->BufferSize.y + ((uint32_t)s_RendererSettings->BufferSize.y % 16)) / 16;
		s_pRenderData->ForwardTileCount = s_RendererSettings->ForwardGroupX * s_RendererSettings->ForwardGroupY;
	}
}