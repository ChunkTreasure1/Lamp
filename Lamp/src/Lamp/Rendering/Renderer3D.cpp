#include "lppch.h"
#include "Renderer3D.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Texture2D/IBLBuffer.h"
#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "UniformBuffer.h"
#include "Lamp/Level/Level.h"
#include "Texture2D/IBLBuffer.h"
#include "RenderGraph/Nodes/DynamicUniformRegistry.h"
#include "RenderBuffer.h"

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
		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVerts = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 2;

		/////Skybox//////
		Ref<Shader> SkyboxShader;
		Ref<VertexArray> SkyboxVertexArray;
		/////////////////

		/////Quad/////	
		Ref<VertexArray> QuadVertexArray;
		//////////////

		//////Lines//////
		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		uint32_t LineIndexCount = 1;

		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
		Material LineMaterial;
		/////////////////

		Ref<CameraBase> Camera;

		Renderer3DStorage()
			: LineMaterial(Lamp::ShaderLibrary::GetShader("Line"), 0)
		{
		}

		~Renderer3DStorage()
		{
			delete[] LineVertexBufferBase;
		}

		RenderPassSpecification* CurrentRenderPass = nullptr;

		/////Shadows/////
		Ref<Shader> DirShadowShader;
		Ref<Framebuffer> ShadowBuffer;
		Ref<Shader> PointShadowShader;
		/////////////////

		Ref<Shader> SelectionShader;

		/////SSAO/////
		std::vector<glm::vec3> SSAONoise;
		std::vector<glm::vec3> SSAOKernel;
		//////////////

		glm::vec2 LastViewportSize = glm::vec2(0.f);

		/////Uniform buffers/////
		struct UBuffer
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::mat4 ShadowVP;
			glm::vec3 CameraPosition;
		} DataBuffer;
		Ref<UniformBuffer> DataUniformBuffer;

		struct SSAOUniformBuffer
		{
			int KernelSize;
			float Radius;
			float Bias;
			glm::vec2 BufferSize;

			glm::vec3 Samplers[64];

		} SSAODataBuffer;
		Ref<UniformBuffer> SSAODataUniformBuffer;
		////////////////////////
	};

	static Renderer3DStorage* s_pData;
	RendererSettings Renderer3D::s_RendererSettings;
	RenderBuffer Renderer3D::s_RenderBuffer;

	static float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pData = new Renderer3DStorage();

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

			s_pData->QuadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(quadVertices, (uint32_t)(sizeof(Vertex) * quadVertices.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			});
			s_pData->QuadVertexArray->AddVertexBuffer(pBuffer);
			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(quadIndices, (uint32_t)quadIndices.size());
			s_pData->QuadVertexArray->SetIndexBuffer(indexBuffer);
		}
		//////////////

		///////Line///////
		{
			s_pData->LineVertexArray = VertexArray::Create();
			s_pData->LineVertexBuffer = VertexBuffer::Create(s_pData->MaxLineVerts * sizeof(LineVertex));
			s_pData->LineVertexBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float4, "a_Color" }
				});
			s_pData->LineVertexArray->AddVertexBuffer(s_pData->LineVertexBuffer);
			s_pData->LineVertexBufferBase = new LineVertex[s_pData->MaxLineVerts];

			uint32_t* pLineIndices = new uint32_t[s_pData->MaxLineIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < s_pData->MaxLineIndices; i += 2)
			{
				pLineIndices[i + 0] = offset + 0;
				pLineIndices[i + 1] = offset + 1;

				offset += 2;
			}

			Ref<IndexBuffer> pLineIB = IndexBuffer::Create(pLineIndices, s_pData->MaxLineIndices);
			s_pData->LineVertexArray->SetIndexBuffer(pLineIB);

			delete[] pLineIndices;
		}
		//////////////////

		/////Shadows/////
		s_pData->DirShadowShader = ShaderLibrary::GetShader("dirShadow");
		s_pData->PointShadowShader = ShaderLibrary::GetShader("pointShadow");
		/////////////////

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

			s_pData->SkyboxVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(boxPositions, (uint32_t)(sizeof(float) * boxPositions.size()));
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" }
			});
			s_pData->SkyboxVertexArray->AddVertexBuffer(pBuffer);

			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(boxIndicies, (uint32_t)(boxIndicies.size()));
			s_pData->SkyboxVertexArray->SetIndexBuffer(indexBuffer);
			s_pData->SkyboxShader = ShaderLibrary::GetShader("Skybox");

			s_RendererSettings.InternalFramebuffers.emplace(std::make_pair("Skybox", CreateRef<IBLBuffer>("assets/textures/Frozen_Waterfall_Ref.hdr")));
		}
		////////////////

		s_pData->SelectionShader = ShaderLibrary::GetShader("selection");

		/////SSAO/////
		RegenerateSSAOKernel();

		Ref<Texture2D> ssaoNoise = Texture2D::Create(4, 4);
		ssaoNoise->SetData(&s_pData->SSAONoise[0], 0);
		s_RendererSettings.InternalTextures.emplace(std::make_pair("SSAONoise", ssaoNoise));
		//////////////

		/////Uniform Buffer/////
		s_pData->DataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DStorage::UBuffer), 0);
		s_pData->SSAODataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DStorage::SSAOUniformBuffer), 1);
		////////////////////////

		//Setup dynamic uniforms
		DynamicUniformRegistry::AddUniform("Exposure", UniformType::Float, RegisterData(&Renderer3D::GetSettings().HDRExposure));
		DynamicUniformRegistry::AddUniform("Gamma", UniformType::Float, RegisterData(&Renderer3D::GetSettings().Gamma));
		DynamicUniformRegistry::AddUniform("Directional light direction", UniformType::Float3, RegisterData(&g_pEnv->DirLight.Position));
		DynamicUniformRegistry::AddUniform("Directional light color", UniformType::Float3, RegisterData(&g_pEnv->DirLight.Color));
		DynamicUniformRegistry::AddUniform("Directional light intensity", UniformType::Float, RegisterData(&g_pEnv->DirLight.Intensity));
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pData;
	}

	void Renderer3D::Begin(Ref<CameraBase>& camera)
	{
		//Main
		{
			s_pData->DataBuffer.CameraPosition = camera->GetPosition();
			s_pData->DataBuffer.Projection = camera->GetProjectionMatrix();
			s_pData->DataBuffer.View = camera->GetViewMatrix();
			s_pData->DataBuffer.ShadowVP = g_pEnv->DirLight.ViewProjection;
			s_pData->DataUniformBuffer->SetData(&s_pData->DataBuffer, sizeof(Renderer3DStorage::DataBuffer));
		}

		//SSAO
		{
			s_pData->SSAODataBuffer.KernelSize = s_RendererSettings.SSAOKernelSize;
			s_pData->SSAODataBuffer.Radius = s_RendererSettings.SSAORadius;
			s_pData->SSAODataBuffer.Bias = s_RendererSettings.SSAOBias;
			s_pData->SSAODataBuffer.BufferSize = s_pData->LastViewportSize;
			s_pData->SSAODataUniformBuffer->SetData(&s_pData->SSAODataBuffer, sizeof(Renderer3DStorage::SSAODataBuffer));
		}

		s_pData->Camera = camera;

		ResetBatchData();
	}

	void Renderer3D::End()
	{
		LP_PROFILE_FUNCTION();
		uint32_t dataSize = (uint8_t*)s_pData->LineVertexBufferPtr - (uint8_t*)s_pData->LineVertexBufferBase;
		s_pData->LineVertexBuffer->SetData(s_pData->LineVertexBufferBase, dataSize);

		s_RenderBuffer.drawCallsDeferred.clear();
		s_RenderBuffer.drawCallsForward.clear();
		Flush();
	}

	void Renderer3D::BeginPass(RenderPassSpecification& passSpec)
	{
		s_pData->CurrentRenderPass = &passSpec;
		s_pData->LastViewportSize = { s_pData->CurrentRenderPass->TargetFramebuffer->GetSpecification().Width, s_pData->CurrentRenderPass->TargetFramebuffer->GetSpecification().Height };
	}

	void Renderer3D::EndPass()
	{
		s_pData->CurrentRenderPass = nullptr;
	}

	void Renderer3D::Flush()
	{
		LP_PROFILE_FUNCTION();
		s_pData->LineMaterial.GetShader()->Bind();

		RenderCommand::DrawIndexedLines(s_pData->LineVertexArray, s_pData->LineIndexCount);
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Ref<VertexArray>& data, Ref<Material> mat, size_t id)
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();
		switch (s_pData->CurrentRenderPass->type)
		{
			case PassType::PointShadow:
			{
				LP_PROFILE_SCOPE("PointShadowPass");
				RenderCommand::SetCullFace(CullFace::Back);
				s_pData->PointShadowShader->Bind();

				uint32_t j = s_pData->CurrentRenderPass->LightIndex;
				const PointLight* light = g_pEnv->pLevel->GetRenderUtils().GetPointLights()[j];

				for (int i = 0; i < light->ShadowBuffer->GetTransforms().size(); i++)
				{
					s_pData->PointShadowShader->UploadMat4("u_Transforms[" + std::to_string(i) + "]", light->ShadowBuffer->GetTransforms()[i]);
				}

				s_pData->PointShadowShader->UploadFloat("u_FarPlane", light->FarPlane);
				s_pData->PointShadowShader->UploadFloat3("u_LightPosition", light->ShadowBuffer->GetPosition());
				s_pData->PointShadowShader->UploadMat4("u_Model", modelMatrix);

				RenderCommand::DrawIndexed(data, data->GetIndexBuffer()->GetCount());

				break;
			}

			default:
			{
				LP_PROFILE_SCOPE("GeometryPass");

				const auto& pass = s_pData->CurrentRenderPass;

				RenderCommand::SetCullFace(pass->cullFace);
				
				Ref<Shader> shaderToUse = pass->renderShader ? pass->renderShader : mat->GetShader();

				shaderToUse->Bind();

				//Static Uniforms
				for (auto[name, type, data] : pass->staticUniforms)
				{
					if (data.type() == typeid(RenderData))
					{
						RenderData type = std::any_cast<RenderData>(data);
						switch (type)
						{
							case Lamp::RenderData::Transform:
								shaderToUse->UploadMat4(name, modelMatrix);
								break;
							case Lamp::RenderData::ID:
								shaderToUse->UploadInt(name, id);
								break;
						}

						continue;
					}

					switch (type)
					{
						case UniformType::Int:
							shaderToUse->UploadInt(name, std::any_cast<int>(data));
							break;

						case UniformType::Float:
							shaderToUse->UploadFloat(name, std::any_cast<float>(data));
							break;

						case UniformType::Float2:
							shaderToUse->UploadFloat2(name, std::any_cast<glm::vec2>(data));
							break;

						case UniformType::Float3:
							shaderToUse->UploadFloat3(name, std::any_cast<glm::vec3>(data));
							break;

						case UniformType::Float4:
							shaderToUse->UploadFloat4(name, std::any_cast<glm::vec4>(data));
							break;

						case UniformType::Mat4:
							shaderToUse->UploadMat4(name, std::any_cast<glm::mat4>(data));
							break;
					}
				}

				//Dynamic Uniforms
				for (auto& [name, type, data] : pass->dynamicUniforms)
				{
					switch (type)
					{
						case UniformType::Int:
							shaderToUse->UploadInt(name, *static_cast<int*>(data));
							break;

						case UniformType::Float:
							shaderToUse->UploadFloat(name, *static_cast<float*>(data));
							break;

						case UniformType::Float3:
							shaderToUse->UploadFloat3(name, *static_cast<glm::vec3*>(data));
							break;

						case UniformType::Float4:
							shaderToUse->UploadFloat4(name, *static_cast<glm::vec4*>(data));
							break;

						case UniformType::Mat4:
							shaderToUse->UploadMat4(name, *static_cast<glm::mat4*>(data));
							break;
					}
				}

				//Framebuffers
				for (auto& [buffer, type, id, texLoc] : pass->framebuffers)
				{
					switch (type)
					{
						case TextureType::Color:
							buffer->BindColorAttachment(id, texLoc);
							break;

						case TextureType::Depth:
							buffer->BindDepthAttachment(id);
							break;

						default:
							break;
					}
				}

				int i = 0;
				for (auto& name : mat->GetShader()->GetSpecifications().TextureNames)
				{
					if (mat->GetTextures()[name].get() != nullptr)
					{
						mat->GetTextures()[name]->Bind(i);
						i++;
					}
				}

				data->Bind();
				RenderCommand::DrawIndexed(data, data->GetIndexBuffer()->GetCount());

				break;
			}
		}
	}

	void Renderer3D::DrawMeshForward(const glm::mat4& modelMatrix, Ref<VertexArray>& data, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		mat->UploadData();

		switch (s_pData->CurrentRenderPass->type)
		{
			case PassType::Forward:
			{
				//Reserve spot 0 for shadow map
				int i = 4;// +g_pEnv->pRenderUtils->GetPointLights().size();
				for (auto& name : mat->GetShader()->GetSpecifications().TextureNames)
				{
					if (mat->GetTextures()[name].get() != nullptr)
					{
						mat->GetTextures()[name]->Bind(i);
						i++;
					}
				}

				mat->GetShader()->Bind();
				mat->GetShader()->UploadMat4("u_Model", modelMatrix);
				mat->GetShader()->UploadFloat("u_Exposure", s_RendererSettings.HDRExposure);
				mat->GetShader()->UploadFloat("u_Gamma", s_RendererSettings.Gamma);

				//mat.GetShader()->UploadInt("u_ShadowMap", 0);
				//s_pData->ShadowBuffer->BindDepthAttachment(0);

				mat->GetShader()->UploadInt("u_IrradianceMap", 1);
				mat->GetShader()->UploadInt("u_PrefilterMap", 2);
				mat->GetShader()->UploadInt("u_BRDFLUT", 3);

				s_RendererSettings.InternalFramebuffers["Skybox"]->BindColorAttachment(1, 0);
				s_RendererSettings.InternalFramebuffers["Skybox"]->BindColorAttachment(2, 1);
				s_RendererSettings.InternalFramebuffers["Skybox"]->BindColorAttachment(3, 2);

				mat->GetShader()->UploadFloat3("u_DirectionalLight.direction", glm::normalize(g_pEnv->DirLight.Position));
				mat->GetShader()->UploadFloat3("u_DirectionalLight.color", g_pEnv->DirLight.Color);
				mat->GetShader()->UploadFloat("u_DirectionalLight.intensity", g_pEnv->DirLight.Intensity);

				for (int i = 0; i < g_pEnv->pLevel->GetRenderUtils().GetPointLights().size(); i++)
				{
					g_pEnv->pLevel->GetRenderUtils().GetPointLights()[i]->ShadowBuffer->BindDepthAttachment(4 + i);
				}

				data->Bind();
				RenderCommand::DrawIndexed(data, data->GetIndexBuffer()->GetCount());

				break;
			}

			case PassType::Selection:
			{
				s_pData->SelectionShader->Bind();
				s_pData->SelectionShader->UploadInt("u_ObjectId", (int)id);
				s_pData->SelectionShader->UploadMat4("u_Model", modelMatrix);

				data->Bind();
				RenderCommand::DrawIndexed(data, data->GetIndexBuffer()->GetCount());
				break;
			}

			default:
				break;
		}
	}

	void Renderer3D::DrawSkybox()
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		RenderCommand::SetCullFace(CullFace::Back);

		LP_PROFILE_FUNCTION();
		s_pData->SkyboxShader->Bind();
		s_pData->SkyboxShader->UploadInt("u_EnvironmentMap", 0);

		s_RendererSettings.InternalFramebuffers["Skybox"]->Bind();

		DrawCube();
	}

	void Renderer3D::DrawCube()
	{
		s_pData->SkyboxVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pData->SkyboxVertexArray, s_pData->SkyboxVertexArray->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::DrawQuad()
	{
		s_pData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_pData->QuadVertexArray, s_pData->QuadVertexArray->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::RenderQuad()
	{
		const auto& pass = s_pData->CurrentRenderPass;
		RenderCommand::SetCullFace(pass->cullFace);
		Ref<Shader> shaderToUse = pass->renderShader; //? pass->renderShader : mat.GetShader();

		shaderToUse->Bind();

		//Static Uniforms
		for (auto& [name, type, data] : pass->staticUniforms)
		{
			switch (type)
			{
				case UniformType::Int:
					shaderToUse->UploadInt(name, std::any_cast<int>(data));
					break;

				case UniformType::Float:
					shaderToUse->UploadFloat(name, std::any_cast<float>(data));
					break;

				case UniformType::Float2:
					shaderToUse->UploadFloat2(name, std::any_cast<glm::vec2>(data));
					break;

				case UniformType::Float3:
					shaderToUse->UploadFloat3(name, std::any_cast<glm::vec3>(data));
					break;

				case UniformType::Float4:
					shaderToUse->UploadFloat4(name, std::any_cast<glm::vec4>(data));
					break;

				case UniformType::Mat4:
					shaderToUse->UploadMat4(name, std::any_cast<glm::mat4>(data));
					break;
			}
		}

		//Dynamic Uniforms
		for (auto& [name, type, data] : pass->dynamicUniforms)
		{
			switch (type)
			{
				case UniformType::Int:
					shaderToUse->UploadInt(name, *static_cast<int*>(data));
					break;

				case UniformType::Float:
					shaderToUse->UploadFloat(name, *static_cast<float*>(data));
					break;

				case UniformType::Float3:
					shaderToUse->UploadFloat3(name, *static_cast<glm::vec3*>(data));
					break;

				case UniformType::Float4:
					shaderToUse->UploadFloat4(name, *static_cast<glm::vec4*>(data));
					break;

				case UniformType::Mat4:
					shaderToUse->UploadMat4(name, *static_cast<glm::mat4*>(data));
					break;
			}
		}

		//Framebuffers
		for (auto& [buffer, type, id, texLoc] : pass->framebuffers)
		{
			switch (type)
			{
				case TextureType::Color:
					buffer->BindColorAttachment(id, texLoc);
					break;

				case TextureType::Depth:
					buffer->BindDepthAttachment(id);
					break;

				default:
					break;
			}
		}

		DrawQuad();
	}

	void Renderer3D::SetEnvironment(const std::string& path)
	{
		s_RendererSettings.InternalFramebuffers["Skybox"] = CreateRef<IBLBuffer>(path);
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh>& mesh, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = mesh->GetVertexArray();

		s_RenderBuffer.drawCallsDeferred.push_back(data);
	}

	void Renderer3D::SubmitMeshForward(const glm::mat4& transform, const Ref<SubMesh>& mesh, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = mesh->GetVertexArray();

		s_RenderBuffer.drawCallsForward.push_back(data);
	}

	void Renderer3D::SubmitQuadForward(const glm::mat4& transform, Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();
		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = s_pData->QuadVertexArray;

		s_RenderBuffer.drawCallsForward.push_back(data);
	}

	void Renderer3D::DrawRenderBuffer()
	{
		LP_PROFILE_FUNCTION();

		if (s_pData->CurrentRenderPass->type == PassType::Forward || s_pData->CurrentRenderPass->type == PassType::Selection)
		{
			for (auto& data : s_RenderBuffer.drawCallsForward)
			{
				DrawMeshForward(data.transform, data.data, data.material, data.id);
			}
		}
		
		if (s_pData->CurrentRenderPass->type != PassType::Forward)
		{
			for (auto& data : s_RenderBuffer.drawCallsDeferred)
			{
				DrawMesh(data.transform, data.data, data.material, data.id);
			}
		}
	}

	void Renderer3D::DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width)
	{
		glLineWidth(width);

		if (s_pData->LineIndexCount >= Renderer3DStorage::MaxLineIndices)
		{
			StartNewBatch();
		}

		s_pData->LineVertexBufferPtr->Position = posA;
		s_pData->LineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->LineVertexBufferPtr++;

		s_pData->LineVertexBufferPtr->Position = posB;
		s_pData->LineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->LineVertexBufferPtr++;

		s_pData->LineIndexCount += 2;
	}

	void Renderer3D::StartNewBatch()
	{
		End();
		ResetBatchData();
	}

	void Renderer3D::ResetBatchData()
	{
		s_pData->LineIndexCount = 0;
		s_pData->LineVertexBufferPtr = s_pData->LineVertexBufferBase;
	}

	void Renderer3D::RegenerateSSAOKernel()
	{
		s_pData->SSAONoise.clear();

		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
		for (uint32_t i = 0; i < s_RendererSettings.SSAOKernelSize; i++)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = float(i) / s_RendererSettings.SSAOKernelSize;

			scale = Lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			s_pData->SSAODataBuffer.Samplers[i] = sample;
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.f);
			s_pData->SSAONoise.push_back(noise);
		}
	}
}