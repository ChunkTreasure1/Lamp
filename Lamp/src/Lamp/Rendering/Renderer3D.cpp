#include "lppch.h"
#include "Renderer3D.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include "TestShader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Lamp
{
    struct Renderer3DStorage
    {
        Ref<VertexArray> pVertexArray;
        Ref<Shader> pShader;
        Ref<Texture2D> pTexture;
    };

    static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
        s_pData = new Renderer3DStorage();
        s_pData->pShader = std::make_shared<Shader>("engine/shaders/shader.vs", "engine/shaders/shader.fs");

        // build and compile our shader zprogram
   // ------------------------------------

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            // positions          // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

        glEnable(GL_DEPTH_TEST);

        s_pData->pVertexArray = VertexArray::Create();
        
        Ref<VertexBuffer> pBuffer;
        pBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
        pBuffer->SetBufferLayout
        ({
            { ElementType::Float3, "a_Position" },
            { ElementType::Float2, "a_TexCoord" }
        });

        s_pData->pVertexArray->AddVertexBuffer(pBuffer);

        s_pData->pTexture = Texture2D::Create("engine/textures/image.jpg");

        // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
        // -------------------------------------------------------------------------------------------

        s_pData->pShader->Bind();
        s_pData->pShader->UploadInt("u_Texture", 0);
	}
	void Renderer3D::Shutdown()
	{
	}
	void Renderer3D::Begin(PerspectiveCamera& camera)
	{
        s_pData->pShader->Bind();
        s_pData->pShader->UploadMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}
	void Renderer3D::End()
	{
	}
	void Renderer3D::TestDraw()
	{
        s_pData->pTexture->Bind();
        s_pData->pShader->Bind();

        // create transformations
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

        s_pData->pShader->UploadMat4("u_Model", model);

        // render container
        s_pData->pVertexArray->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}