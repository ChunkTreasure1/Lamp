#include <Lamp.h>

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		static Lamp::GLTexture texture = Lamp::ResourceManager::GetTexture("Textures/ff.png");

		glm::vec4 pos(0, 0, 50, 50);

		Lamp::Sprite sprite(pos, texture.Id, 0.f);
		s_pRenderer->AddSprite(sprite);
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}