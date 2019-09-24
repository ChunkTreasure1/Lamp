#include <Lamp.h>

class RenderLayer : public Lamp::Layer
{
public:
	RenderLayer()
		: Layer("Render Layer"), m_CameraController(1280 / 720)
	{ 
		m_pShader.reset(new Lamp::Shader("Shaders/colorShading.vert", "Shaders/colorShading.frag"));

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		static Lamp::GLTexture texture = Lamp::ResourceManager::GetTexture("Textures/vlad.PNG");
		glm::vec4 pos(10, 10, 50, 50);
		sprite = new Lamp::Sprite(pos, texture.Id, 0.f);
	}

	void Update(Lamp::Timestep ts) override
	{
		m_CameraController.Update(ts);

		Lamp::Renderer::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });
		Lamp::Renderer::Clear();

		Lamp::Renderer::Begin(m_CameraController.GetCamera());

		Lamp::Renderer::Draw(m_pShader, *sprite);

		Lamp::Renderer::End();
	}

	void OnEvent(Lamp::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

private:
	Lamp::OrthographicCameraController m_CameraController;
	std::shared_ptr<Lamp::Shader> m_pShader;
	Lamp::Sprite* sprite;
};

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new RenderLayer());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}