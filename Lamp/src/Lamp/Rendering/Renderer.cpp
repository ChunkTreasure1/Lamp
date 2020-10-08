#include "lppch.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "Renderer3D.h"

#include "RenderCommand.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;

	void Renderer::Initialize()
	{
		RenderCommand::Initialize();
		//Renderer2D::Initialize();
		Renderer3D::Initialize();
	}

	void Renderer::Shutdown()
	{
		//Renderer2D::Shutdown();
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