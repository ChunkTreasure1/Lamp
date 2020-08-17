#include <Lamp/Core/Game.h>

#include <Lamp/Physics/Collision.h>
#include <Lamp/Objects/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

#include <Lamp/Physics/Colliders/BoundingSphere.h>
#include <Lamp/Physics/Colliders/AABB.h>
#include <Lamp/Physics/PhysicsEngine.h>
#include <Lamp/Physics/Physics.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>
#include <Lamp/Meshes/GeometrySystem.h>

#include <Lamp/Objects/ObjectLayer.h>
#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>

class Game : public Lamp::GameBase
{
public:
	Game()
	{}

	virtual void OnStart() override;
	bool OnUpdate(Lamp::AppUpdateEvent& e);
	virtual void OnEvent(Lamp::Event& e) override;
};