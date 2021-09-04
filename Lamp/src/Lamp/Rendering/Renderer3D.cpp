#include "lppch.h"
#include "Renderer3D.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Texture2D/IBLBuffer.h"
#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

#include <random>

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
		Ref<IBLBuffer> SkyboxBuffer;
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

		/////Grid/////
		Ref<VertexArray> GridVertexArray;
		Ref<Shader> GridShader;
		//////////////

		Renderer3DStorage()
			: LineMaterial(Lamp::ShaderLibrary::GetShader("Line"), 0)
		{}

		~Renderer3DStorage()
		{
			delete[] LineVertexBufferBase;
		}

		RenderPassSpecification* CurrentRenderPass = nullptr;
		Ref<VertexArray> SphereArray;

		/////Shadows/////
		Ref<Shader> DirShadowShader;
		Ref<Framebuffer> ShadowBuffer;
		Ref<Shader> PointShadowShader;
		/////////////////

		/////SSAO/////
		Ref<Shader> SSAODepthShader;
		Ref<Shader> SSAOMainShader;
		Ref<Framebuffer> SSOADepthbuffer;
		std::vector<glm::vec3> SSAOKernel;
		//////////////
	};

	static Renderer3DStorage* s_pData;

	static float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	void Renderer3D::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pData = new Renderer3DStorage();

		/////Quad/////
		{
			std::vector<float> quadPositions =
			{
				// positions       // texture Coords
				-1.f, -1.f, 0.0f,  0.0f, 0.0f,
				 1.f, -1.f, 0.0f,  1.0f, 0.0f,
				 1.f,  1.f, 0.0f,  1.0f, 1.0f,
				-1.f,  1.f, 0.0f,  0.0f, 1.0f,
			};

			std::vector<uint32_t> quadIndices =
			{
				0, 1, 2,
				0, 2, 3
			};

			s_pData->QuadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(quadPositions, (uint32_t)sizeof(float) * quadPositions.size());
			pBuffer->SetBufferLayout
			({
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float2, "a_TexCoords" }
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
			s_pData->SkyboxBuffer = CreateRef<IBLBuffer>("assets/textures/Frozen_Waterfall_Ref.hdr");
		}
		////////////////

		/////SSAO//////
		s_pData->SSAODepthShader = ShaderLibrary::GetShader("ssaoDepth");
		s_pData->SSAOMainShader = ShaderLibrary::GetShader("ssaoMain");
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0;

			// scale samples s.t. they're more aligned to center of kernel
			scale = Lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}

		s_pData->SSAOKernel = ssaoKernel;
		///////////////
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pData;
	}

	void Renderer3D::Begin(RenderPassSpecification& passSpec)
	{
		s_pData->CurrentRenderPass = &passSpec;

		ResetBatchData();
	}

	void Renderer3D::End()
	{
		uint32_t dataSize = (uint8_t*)s_pData->LineVertexBufferPtr - (uint8_t*)s_pData->LineVertexBufferBase;
		s_pData->LineVertexBuffer->SetData(s_pData->LineVertexBufferBase, dataSize);

		Flush();
		s_pData->CurrentRenderPass = nullptr;
	}

	void Renderer3D::Flush()
	{
		s_pData->LineMaterial.GetShader()->Bind();
		s_pData->LineMaterial.GetShader()->UploadMat4("u_ViewProjection", s_pData->CurrentRenderPass->Camera->GetViewProjectionMatrix());

		RenderCommand::DrawIndexedLines(s_pData->LineVertexArray, s_pData->LineIndexCount);
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id)
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();
		switch (s_pData->CurrentRenderPass->type)
		{
			case PassType::DirectionalShadow:
			{
				/////Directional light shadows/////
				glCullFace(GL_FRONT);
				s_pData->DirShadowShader->Bind();

				glm::mat4 shadowMVP = g_pEnv->DirLight.ViewProjection * modelMatrix;
				s_pData->DirShadowShader->UploadMat4("u_ShadowMVP", shadowMVP);
				s_pData->ShadowBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

				mesh->GetVertexArray()->Bind();
				RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
				break;
				////////////////////////////////////
			}

			case PassType::PointShadow:
			{
				///////Point light shadows/////
				glCullFace(GL_BACK);
				s_pData->PointShadowShader->Bind();

				uint32_t j = s_pData->CurrentRenderPass->LightIndex;
				const PointLight* light = g_pEnv->pRenderUtils->GetPointLights()[j];

				for (int i = 0; i < light->ShadowBuffer->GetTransforms().size(); i++)
				{
					s_pData->PointShadowShader->UploadMat4("u_Transforms[" + std::to_string(i) + "]", light->ShadowBuffer->GetTransforms()[i]);
				}

				s_pData->PointShadowShader->UploadFloat("u_FarPlane", light->FarPlane);
				s_pData->PointShadowShader->UploadFloat3("u_LightPosition", light->ShadowBuffer->GetPosition());
				s_pData->PointShadowShader->UploadMat4("u_Model", modelMatrix);

				RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
				break;
				///////////////////////////////
			}

			case PassType::SSAODepth:
			{
				s_pData->SSOADepthbuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

				s_pData->SSAODepthShader->Bind();
				s_pData->SSAODepthShader->UploadMat4("u_ViewProjection", s_pData->CurrentRenderPass->Camera->GetViewProjectionMatrix());
				s_pData->SSAODepthShader->UploadMat4("u_Model", modelMatrix);
				s_pData->SSAODepthShader->UploadMat4("u_View", s_pData->CurrentRenderPass->Camera->GetViewMatrix());

				mesh->GetVertexArray()->Bind();
				RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());

				break;
			}

			case PassType::SSAOMain:
			{

				s_pData->SSAOMainShader->Bind();

				s_pData->SSAOMainShader->UploadInt("u_DepthMap", 0);
				s_pData->SSOADepthbuffer->BindDepthAttachment(0);

				s_pData->SSAOMainShader->UploadMat4("u_Projection", s_pData->CurrentRenderPass->Camera->GetProjectionMatrix());
				for (uint32_t i = 0; i < 64; i++)
				{
					s_pData->SSAOMainShader->UploadFloat3("u_Samples[" + std::to_string(i) + "]", s_pData->SSAOKernel[i]);
				}
				break;
			}

			case PassType::Main:
			{
				glCullFace(GL_BACK);
				//Reserve spot 0 for shadow map
				int i = 4;// +g_pEnv->pRenderUtils->GetPointLights().size();
				for (auto& name : mat.GetShader()->GetSpecifications().TextureNames)
				{
					if (mat.GetTextures()[name].get() != nullptr)
					{
						mat.GetTextures()[name]->Bind(i);
						i++;
					}
				}

				mat.GetShader()->Bind();
				mat.GetShader()->UploadFloat3("u_CameraPosition", s_pData->CurrentRenderPass->Camera->GetPosition());
				mat.GetShader()->UploadMat4("u_Model", modelMatrix);
				mat.GetShader()->UploadMat4("u_ViewProjection", s_pData->CurrentRenderPass->Camera->GetViewProjectionMatrix());
				mat.GetShader()->UploadMat4("u_SunShadowMVP", g_pEnv->DirLight.ViewProjection * modelMatrix);
				mat.GetShader()->UploadInt("u_ObjectId", id);
				mat.GetShader()->UploadFloat("u_Exposure", g_pEnv->HDRExposure);

				mat.GetShader()->UploadInt("u_ShadowMap", 0);
				s_pData->ShadowBuffer->BindDepthAttachment(0);

				mat.GetShader()->UploadInt("u_IrradianceMap", 1);
				mat.GetShader()->UploadInt("u_PrefilterMap", 2);
				mat.GetShader()->UploadInt("u_BRDFLUT", 3);

				s_pData->SkyboxBuffer->BindTextures(1);

				for (int i = 0; i < g_pEnv->pRenderUtils->GetPointLights().size(); i++)
				{
					g_pEnv->pRenderUtils->GetPointLights()[i]->ShadowBuffer->BindDepthAttachment(4 + i);
				}

				mesh->GetVertexArray()->Bind();
				RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
				break;
			}

			default:
				break;
		}
	}

	void Renderer3D::DrawSkybox()
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();
		s_pData->SkyboxShader->Bind();
		s_pData->SkyboxShader->UploadMat4("u_View", s_pData->CurrentRenderPass->Camera->GetViewMatrix());
		s_pData->SkyboxShader->UploadMat4("u_Projection", s_pData->CurrentRenderPass->Camera->GetProjectionMatrix());
		s_pData->SkyboxShader->UploadInt("u_EnvironmentMap", 0);

		s_pData->SkyboxBuffer->Bind();

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

	void Renderer3D::DrawGrid()
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		s_pData->GridShader->Bind();
		s_pData->GridVertexArray->Bind();

		glm::mat4 viewMat = glm::mat4(glm::mat3(s_pData->CurrentRenderPass->Camera->GetViewMatrix()));
		s_pData->GridShader->UploadMat4("u_View", viewMat);

		s_pData->GridShader->UploadMat4("u_Projection", s_pData->CurrentRenderPass->Camera->GetViewProjectionMatrix());
		RenderCommand::DrawIndexed(s_pData->GridVertexArray, 0);
	}

	void Renderer3D::SetEnvironment(const std::string& path)
	{
		s_pData->SkyboxBuffer = CreateRef<IBLBuffer>(path);
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
}