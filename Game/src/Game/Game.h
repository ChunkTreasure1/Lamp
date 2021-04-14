#include <Lamp/Core/Game.h>

#include <Lamp/Objects/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

class Game : public Lamp::GameBase
{
public:
	Game()
	{}

	~Game();

	virtual void OnStart() override;
	bool OnUpdate(Lamp::AppUpdateEvent& e);
	virtual void OnEvent(Lamp::Event& e) override;

private:
	Lamp::Level* m_pLevel = nullptr;
};