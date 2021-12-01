#include "lppch.h"
#include "Renderer3D.h"

#include "Lamp/Core/Application.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Lamp/Level/Level.h"

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
		
		const uint32_t maxShadowCasters = 5;
		std::vector<PointLight*> pointLightsToUse;
	};

	static Renderer3DStorage* s_pRenderData;
	RenderBuffer Renderer3D::s_opaqueRenderBuffer;
	RenderBuffer Renderer3D::s_transparentRenderBuffer;
	
	Renderer3D::Statistics Renderer3D::s_renderStatistics;

	void Renderer3D::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pRenderData = new Renderer3DStorage();

		CreateBaseMeshes();

		s_opaqueRenderBuffer.drawCalls.reserve(1000);
		s_transparentRenderBuffer.drawCalls.reserve(500);
	}

	void Renderer3D::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pRenderData;
	}

	void Renderer3D::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		s_pRenderData->camera = camera;
		s_renderStatistics.totalDrawCalls = 0;
		s_renderStatistics.otherDrawCalls = 0;
		s_renderStatistics.sceneDrawCalls = 0;

		s_pRenderData->pointLightsToUse.clear();

		DrawDirectionalShadows();
		DrawPointShadows();

		SortBuffers(camera->GetPosition());
	}

	void Renderer3D::End()
	{
		LP_PROFILE_FUNCTION();
		s_opaqueRenderBuffer.drawCalls.clear();
		s_transparentRenderBuffer.drawCalls.clear();
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

		DrawQuad();
	}

	void Renderer3D::SetEnvironment(const std::string& path)
	{
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> mat, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData data;
		data.transform = transform;
		data.material = mat;
		data.id = id;
		data.data = mesh->GetVertexArray();

		if (mat->GetUseBlending())
		{
			s_transparentRenderBuffer.drawCalls.push_back(data);
		}
		else
		{
			s_opaqueRenderBuffer.drawCalls.push_back(data);
		}
	}

	void Renderer3D::DrawRenderBuffer()
	{
		LP_PROFILE_FUNCTION();

		for (const auto& data : s_opaqueRenderBuffer.drawCalls)
		{
			DrawMesh(data.transform, data.data, data.material, data.id);
		}

		for (const auto& data : s_transparentRenderBuffer.drawCalls)
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

	void Renderer3D::SortBuffers(const glm::vec3& sortPoint)
	{
		std::sort(s_opaqueRenderBuffer.drawCalls.begin(), s_opaqueRenderBuffer.drawCalls.end(), [&sortPoint](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distOne = glm::exp2(sortPoint.x - dPosOne.x) + glm::exp2(sortPoint.y - dPosOne.y) + glm::exp2(sortPoint.z - dPosOne.z);
				const float distTwo = glm::exp2(sortPoint.x - dPosTwo.x) + glm::exp2(sortPoint.y - dPosTwo.y) + glm::exp2(sortPoint.z - dPosTwo.z);

				return distOne < distTwo;
			});

		std::sort(s_transparentRenderBuffer.drawCalls.begin(), s_transparentRenderBuffer.drawCalls.end(), [&sortPoint](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distOne = glm::exp2(sortPoint.x - dPosOne.x) + glm::exp2(sortPoint.y - dPosOne.y) + glm::exp2(sortPoint.z - dPosOne.z);
				const float distTwo = glm::exp2(sortPoint.x - dPosTwo.x) + glm::exp2(sortPoint.y - dPosTwo.y) + glm::exp2(sortPoint.z - dPosTwo.z);

				return distOne < distTwo;
			});
	}

	void Renderer3D::DrawPointShadows()
	{
		LP_PROFILE_FUNCTION();

		const glm::vec3 cameraPosition = s_pRenderData->camera->GetPosition();

		auto& pointLights = g_pEnv->pLevel->GetRenderUtils().GetPointLights();
		uint32_t shadowCasterCount = std::min((uint32_t)pointLights.size(), s_pRenderData->maxShadowCasters);
		s_pRenderData->pointLightsToUse.reserve(shadowCasterCount);

		//Sort lights by distance to camera
		s_pRenderData->pointLightsToUse.insert(s_pRenderData->pointLightsToUse.end(), pointLights.begin(), pointLights.end());
		//std::sort(s_pRenderData->pointLightsToUse.begin(), s_pRenderData->pointLightsToUse.end(), [&cameraPosition](PointLight* first, PointLight* second)
		//	{
		//		const glm::vec3& dPosOne = first->shadowBuffer->GetPosition();
		//		const glm::vec3& dPosTwo = second->shadowBuffer->GetPosition();
		//
		//		const float distOne = glm::exp2(cameraPosition.x - dPosOne.x) + glm::exp2(cameraPosition.y - dPosOne.y) + glm::exp2(cameraPosition.z - dPosOne.z);
		//		const float distTwo = glm::exp2(cameraPosition.x - dPosTwo.x) + glm::exp2(cameraPosition.y - dPosTwo.y) + glm::exp2(cameraPosition.z - dPosTwo.z);
		//
		//		return distOne < distTwo;
		//	});

		////Render shadows for nearest
		//for (uint32_t i = 0; i < shadowCasterCount; i++)
		//{
		//	s_pRenderData->pointLightsToUse[i]->shadowBuffer->Bind();
		//	RenderCommand::ClearDepth();

		//	SortBuffers(s_pRenderData->pointLightsToUse[i]->shadowBuffer->GetPosition());
		//	BeginPass(s_pRenderData->pointLightsToUse[i]->shadowPass->GetSpecification());

		//	DrawRenderBuffer();

		//	EndPass();
		//	s_pRenderData->pointLightsToUse[i]->shadowBuffer->Unbind();
		//}

		//Upload data
		auto& sceneData = Renderer::s_pSceneData;

		PointLightData* buffer = (PointLightData*)sceneData->pointLightStorageBuffer->Map();
		sceneData->pointLightCount = 0;

		for (uint32_t i = 0; i < s_pRenderData->pointLightsToUse.size(); i++) 
		{
			const auto& light = s_pRenderData->pointLightsToUse[i];

			buffer[i].position = glm::vec4(light->shadowBuffer->GetPosition(), 0.f); //TODO: Change to using other position
			buffer[i].color = glm::vec4(light->color, 0.f);
			buffer[i].intensity = light->intensity;
			buffer[i].falloff = light->falloff;
			buffer[i].farPlane = light->farPlane;
			buffer[i].radius = light->radius;
			buffer[i].samplerId = i;

			sceneData->pointLightCount++;
		}

		sceneData->pointLightStorageBuffer->Unmap();
	}

	void Renderer3D::DrawDirectionalShadows()
	{
		LP_PROFILE_FUNCTION();

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
}