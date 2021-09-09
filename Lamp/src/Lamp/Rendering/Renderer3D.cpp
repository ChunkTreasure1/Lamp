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
#include "UniformBuffer.h"

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

		Ref<Shader> GBufferShader;
		Ref<Shader> DeferredShader;
		Ref<Shader> SelectionShader;
		Ref<Framebuffer> GBuffer;
		Ref<Framebuffer> LightBuffer;

		/////SSAO/////
		Ref<Texture2D> SSAONoiseTexture;
		std::vector<glm::vec3> SSAONoise;
		std::vector<glm::vec3> SSAOKernel;
		Ref<Shader> SSAOMainShader;
		Ref<Shader> SSAOBlurShader;
		Ref<Framebuffer> SSAOBuffer;
		Ref<Framebuffer> SSAOBlurBuffer;
		//////////////

		/////Uniform buffer/////
		struct UBuffer
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::mat4 ShadowVP;
			glm::vec3 CameraPosition;
		} DataBuffer;
		Ref<UniformBuffer> DataUniformBuffer;
		////////////////////////
	};

	static Renderer3DStorage* s_pData;
	RendererSettings Renderer3D::s_RendererSettings;

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
			std::vector<float> quadVertices =
			{
				// positions       // texture Coords
				 1.f,  1.f, 0.0f,  1.0f, 1.0f, // top right
				 1.f, -1.f, 0.0f,  1.0f, 0.0f, // bottom right
				-1.f, -1.f, 0.0f,  0.0f, 0.0f, // bottom left
				-1.f,  1.f, 0.0f,  0.0f, 1.0f, // top left
			};

			std::vector<uint32_t> quadIndices =
			{
				0, 1, 3, //(top left - bottom left - top right)
				1, 2, 3 //(top left - top right - bottom right)
			};

			s_pData->QuadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> pBuffer = VertexBuffer::Create(quadVertices, (uint32_t)sizeof(float) * quadVertices.size());
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

		s_pData->GBufferShader = ShaderLibrary::GetShader("gbuffer");
		s_pData->DeferredShader = ShaderLibrary::GetShader("deferred");
		s_pData->SelectionShader = ShaderLibrary::GetShader("selection");

		/////SSAO/////
		RegenerateSSAOKernel();

		s_pData->SSAONoiseTexture = Texture2D::Create(4, 4);
		s_pData->SSAONoiseTexture->SetData(&s_pData->SSAONoise[0], 0);

		s_pData->SSAOMainShader = ShaderLibrary::GetShader("SSAOMain");
		s_pData->SSAOBlurShader = ShaderLibrary::GetShader("SSAOBlur");
		//////////////

		/////Uniform Buffer/////
		s_pData->DataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DStorage::UBuffer), 0);
		////////////////////////
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pData;
	}

	void Renderer3D::Begin(RenderPassSpecification& passSpec)
	{
		s_pData->CurrentRenderPass = &passSpec;

		s_pData->DataBuffer.CameraPosition = passSpec.Camera->GetPosition();
		s_pData->DataBuffer.Projection = passSpec.Camera->GetProjectionMatrix();
		s_pData->DataBuffer.View = passSpec.Camera->GetViewMatrix();
		s_pData->DataBuffer.ShadowVP = g_pEnv->DirLight.ViewProjection;

		s_pData->DataUniformBuffer->SetData(&s_pData->DataBuffer, sizeof(Renderer3DStorage::DataBuffer));

		ResetBatchData();
	}

	void Renderer3D::End()
	{
		uint32_t dataSize = (uint8_t*)s_pData->LineVertexBufferPtr - (uint8_t*)s_pData->LineVertexBufferBase;
		s_pData->LineVertexBuffer->SetData(s_pData->LineVertexBufferBase, dataSize);

		Flush();

		s_pData->CurrentRenderPass = nullptr;
	}

	void Renderer3D::CombineLightning()
	{
		if (!s_pData->GBuffer || !s_pData->SSAOBuffer)
		{
			return;
		}

		RenderCommand::SetCullFace(CullFace::Front);
		s_pData->DeferredShader->Bind();

		s_pData->DeferredShader->UploadInt("u_GBuffer.position", 0);
		s_pData->DeferredShader->UploadInt("u_GBuffer.normal", 1);
		s_pData->DeferredShader->UploadInt("u_GBuffer.albedo", 2);

		s_pData->DeferredShader->UploadInt("u_ShadowMap", 3);
		s_pData->DeferredShader->UploadInt("u_IrradianceMap", 4);
		s_pData->DeferredShader->UploadInt("u_PrefilterMap", 5);
		s_pData->DeferredShader->UploadInt("u_BRDFLUT", 6);
		s_pData->DeferredShader->UploadInt("u_SSAO", 7);

		s_pData->GBuffer->BindColorAttachment(0, 0);
		s_pData->GBuffer->BindColorAttachment(1, 1);
		s_pData->GBuffer->BindColorAttachment(2, 2);
		s_pData->ShadowBuffer->BindDepthAttachment(3);
		s_pData->SkyboxBuffer->BindTextures(4);

		s_pData->SSAOBlurBuffer->BindColorAttachment(7);

		s_pData->DeferredShader->UploadFloat("u_Exposure", s_RendererSettings.HDRExposure);
		s_pData->DeferredShader->UploadFloat("u_Gamma", s_RendererSettings.Gamma);
		
		s_pData->DeferredShader->UploadFloat3("u_DirectionalLight.direction", glm::normalize(g_pEnv->DirLight.Position));
		s_pData->DeferredShader->UploadFloat3("u_DirectionalLight.color", g_pEnv->DirLight.Color);
		s_pData->DeferredShader->UploadFloat("u_DirectionalLight.intensity", g_pEnv->DirLight.Intensity);


		int lightCount = 0;
		for (auto& light : g_pEnv->pRenderUtils->GetPointLights())
		{
			if (lightCount > 11)
			{
				LP_CORE_WARN("There are more lights in scene than able to render! Will skip some lights.");
				break;
			}

			std::string v = std::to_string(lightCount);

			s_pData->DeferredShader->UploadFloat("u_PointLights[" + v + "].intensity", light->Intensity);
			s_pData->DeferredShader->UploadFloat("u_PointLights[" + v + "].radius", light->Radius);
			s_pData->DeferredShader->UploadFloat("u_PointLights[" + v + "].falloff", light->Falloff);
			s_pData->DeferredShader->UploadFloat("u_PointLights[" + v + "].farPlane", light->FarPlane);

			s_pData->DeferredShader->UploadFloat3("u_PointLights[" + v + "].position", light->ShadowBuffer->GetPosition());
			s_pData->DeferredShader->UploadFloat3("u_PointLights[" + v + "].color", light->Color);
			s_pData->DeferredShader->UploadInt("u_PointLights[" + v + "].shadowMap", lightCount + 8);

			light->ShadowBuffer->BindDepthAttachment(8 + lightCount);

			lightCount++;
		}

		for (int i = 0; i < 12; i++)
		{
			s_pData->DeferredShader->UploadInt("u_PointLights[" + std::to_string(i) + "].shadowMap", 8 + i);
		}

		s_pData->LightBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;
		s_pData->DeferredShader->UploadInt("u_LightCount", lightCount);

		DrawQuad();
	}

	void Renderer3D::Flush()
	{
		s_pData->LineMaterial.GetShader()->Bind();

		RenderCommand::DrawIndexedLines(s_pData->LineVertexArray, s_pData->LineIndexCount);
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id)
	{
		LP_ASSERT(s_pData->CurrentRenderPass != nullptr, "Has Renderer3D::Begin been called?");

		LP_PROFILE_FUNCTION();
		switch (s_pData->CurrentRenderPass->type)
		{
		case PassType::DirShadow:
		{
			RenderCommand::SetCullFace(CullFace::Front);
			s_pData->DirShadowShader->Bind();

			s_pData->DirShadowShader->UploadMat4("u_Model", modelMatrix);
			s_pData->ShadowBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

			mesh->GetVertexArray()->Bind();
			RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());

			break;
		}

		case PassType::PointShadow:
		{
			RenderCommand::SetCullFace(CullFace::Back);
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
		}

		case PassType::Selection:
		{
			s_pData->SelectionShader->Bind();
			s_pData->SelectionShader->UploadInt("u_ObjectId", id);
			s_pData->SelectionShader->UploadMat4("u_Model", modelMatrix);

			mesh->GetVertexArray()->Bind();
			RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
			break;
		}

		case PassType::Geometry:
		{
			RenderCommand::SetCullFace(CullFace::Back);
			s_pData->GBufferShader->Bind();
			s_pData->GBufferShader->UploadMat4("u_Model", modelMatrix);

			s_pData->GBufferShader->UploadInt("u_Material.albedo", 0);
			s_pData->GBufferShader->UploadInt("u_Material.normal", 1);
			s_pData->GBufferShader->UploadInt("u_Material.mro", 2);

			int i = 0;
			for (auto& name : mat.GetShader()->GetSpecifications().TextureNames)
			{
				if (mat.GetTextures()[name].get() != nullptr)
				{
					mat.GetTextures()[name]->Bind(i);
					i++;
				}
			}

			mesh->GetVertexArray()->Bind();
			RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());

			s_pData->GBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

			break;
		}

		}
	}

	void Renderer3D::DrawMeshForward(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id)
	{
		RenderCommand::SetCullFace(CullFace::Back);

		switch (s_pData->CurrentRenderPass->type)
		{
		case PassType::Forward:
		{
			//Reserve spot 0 for shadow map
			int i = 4;// +g_pEnv->pRenderUtils->GetPointLights().size();
			for (auto& name : mat.GetShader()->GetSpecifications().TextureNames)
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
				mat.GetShader()->UploadMat4("u_Model", modelMatrix);
				mat.GetShader()->UploadInt("u_ObjectId", id);
				mat.GetShader()->UploadFloat("u_Exposure", s_RendererSettings.HDRExposure);
				mat.GetShader()->UploadFloat("u_Gamma", s_RendererSettings.Gamma);

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

			mesh->GetVertexArray()->Bind();
			RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
			break;
		}

		case PassType::Selection:
		{
			s_pData->SelectionShader->Bind();
			s_pData->SelectionShader->UploadInt("u_ObjectId", id);
			s_pData->SelectionShader->UploadMat4("u_Model", modelMatrix);

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

		RenderCommand::SetCullFace(CullFace::Back);

		LP_PROFILE_FUNCTION();
		s_pData->SkyboxShader->Bind();
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

	void Renderer3D::CopyDepth()
	{
		if (!s_pData->GBuffer || !s_pData->LightBuffer)
		{
			return;
		}

		s_pData->LightBuffer->Copy(s_pData->GBuffer->GetRendererID(), { s_pData->LightBuffer->GetSpecification().Width, s_pData->LightBuffer->GetSpecification().Height }, true);
	}

	void Renderer3D::SSAOMainPass()
	{
		if (!s_pData->GBuffer)
		{
			return;
		}

		RenderCommand::SetCullFace(CullFace::Front);

		s_pData->SSAOMainShader->Bind();

		for (uint32_t i = 0; i < s_pData->SSAOKernel.size(); i++)
		{
			s_pData->SSAOMainShader->UploadFloat3("u_Samples[" + std::to_string(i) + "]", s_pData->SSAOKernel[i]);
		}

		s_pData->SSAOMainShader->UploadInt("u_GBuffer.position", 0);
		s_pData->SSAOMainShader->UploadInt("u_GBuffer.normal", 1);
		s_pData->SSAOMainShader->UploadInt("u_KernelSize", s_RendererSettings.SSAOKernelSize);
		s_pData->SSAOMainShader->UploadFloat("u_Radius", s_RendererSettings.SSAORadius);
		s_pData->SSAOMainShader->UploadFloat("u_Bias", s_RendererSettings.SSAOBias);

		s_pData->SSAOMainShader->UploadInt("u_Noise", 2);

		glm::vec2 bufferSize = { s_pData->CurrentRenderPass->TargetFramebuffer->GetSpecification().Width, s_pData->CurrentRenderPass->TargetFramebuffer->GetSpecification().Height };
		s_pData->SSAOMainShader->UploadFloat2("u_BufferSize", bufferSize);

		s_pData->GBuffer->BindColorAttachment(0, 0);
		s_pData->GBuffer->BindColorAttachment(1, 1);
		s_pData->SSAONoiseTexture->Bind(2);

		s_pData->SSAOBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

		DrawQuad();
	}

	void Renderer3D::SSAOBlurPass()
	{
		if (!s_pData->SSAOBuffer)
		{
			return;
		}

		RenderCommand::SetCullFace(CullFace::Front);

		s_pData->SSAOBlurShader->Bind();
		s_pData->SSAOBlurShader->UploadInt("u_SSAO", 0);
		s_pData->SSAOBuffer->BindColorAttachment(0);

		s_pData->SSAOBlurBuffer = s_pData->CurrentRenderPass->TargetFramebuffer;

		DrawQuad();
	}

	void Renderer3D::RegenerateSSAOKernel()
	{
		s_pData->SSAOKernel.clear();
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
			s_pData->SSAOKernel.push_back(sample);
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.f);
			s_pData->SSAONoise.push_back(noise);
		}
	}
}