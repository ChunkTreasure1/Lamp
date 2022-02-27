#pragma once

#include "Lamp/Objects/Entity/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Entity.h"

#include "Lamp/Mesh/Mesh.h"

namespace Lamp
{
	class MeshInstance;
	class MeshComponent final : public EntityComponent
	{
	public:
		MeshComponent();
		~MeshComponent();

		//////Base//////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		////////////////

		static Ref<EntityComponent> Create() { return CreateRef<MeshComponent>(); }
		static std::string GetFactoryName() { return "MeshComponent"; }

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnPropertyChanged(ObjectPropertyChangedEvent& e);

		Ref<MeshInstance> m_mesh;
		std::string m_path;
	};

}