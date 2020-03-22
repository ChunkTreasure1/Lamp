#include "lppch.h"
#include "Renderer3D.h"
#include "Renderer.h"

#include "Vertices/VertexArray.h"

namespace Lamp
{
	struct Renderer3DStorage
	{
		Ref<VertexArray> pModelVertexArray;
	};

	void Renderer3D::Initialize()
	{
	}

	void Renderer3D::Shutdown()
	{
	}

	void Renderer3D::Begin(const OrthographicCamera camera)
	{
	}

	void Renderer3D::End()
	{
	}

	void Renderer3D::DrawModel(const glm::vec3& pos, const glm::vec3 scale)
	{
	}

}