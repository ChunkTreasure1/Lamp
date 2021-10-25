#include <Lamp/Core/Game.h>

#include <Lamp/Event/ApplicationEvent.h>

class Game : public Lamp::GameBase
{
public:
	Game()
	{}

	virtual void OnStart() override;
	virtual void OnStop() override;

	bool OnUpdate(Lamp::AppUpdateEvent& e);
	virtual void OnEvent(Lamp::Event& e) override;

private:
};