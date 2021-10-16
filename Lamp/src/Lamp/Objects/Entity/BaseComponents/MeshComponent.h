#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Mesh/Mesh.h"

namespace Lamp
{
	class MeshComponent final : public EntityComponent
	{
	public:
		MeshComponent()
			: EntityComponent("MeshComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Path, "Path", RegisterData(&m_Path) }
			});
		}

		~MeshComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		////////////////

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnPropertyChanged(ObjectPropertyChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<MeshComponent>(); }
		static std::string GetFactoryName() { return "MeshComponent"; }

	private:
		Ref<Mesh> m_Model;
		std::string m_Path;
	};

}