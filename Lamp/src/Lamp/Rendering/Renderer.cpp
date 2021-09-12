#include "lppch.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "Renderer3D.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Meshes/Materials/MaterialLibrary.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;

	void Renderer::Initialize()
	{
		LP_PROFILE_FUNCTION();
		ShaderLibrary::LoadShaders();
		MaterialLibrary::LoadMaterials();

		RenderCommand::Initialize();
		Renderer3D::Initialize();
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		Renderer3D::Shutdown();
	}

	void Renderer::Begin(OrthographicCamera & camera)
	{
		s_pSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::End()
	{
	}
}