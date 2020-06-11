#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class MeshComponent final : public EntityComponent
	{
	public:

		MeshComponent()
			: EntityComponent("MeshComponent")
		{}

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		////////////////

		//Setting
		inline void SetModel(Ref<Model> model) 
		{ 
			m_Model = model; 
			//TODO: change to file dialog instead
			SetComponentProperties
			({
				{ PropertyType::String, "Path", static_cast<void*>(&m_Model->GetLGFPath())}
			});
		}

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);

	public:
		static Ref<EntityComponent> Create() { return std::make_shared<MeshComponent>(); }
		static std::string GetFactoryName() { return "MeshComponent"; }

	private:
		static bool s_Registered;

	private:
		Ref<Model> m_Model;
	};
}