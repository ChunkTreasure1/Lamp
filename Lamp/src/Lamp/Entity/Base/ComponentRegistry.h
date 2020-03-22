#pragma once

#include "BaseComponent.h"

#define LP_REGISTER_COMPONENT(x) Lamp::ComponentRegistry::Register(x::GetFactoryName(), x::Create);

namespace Lamp
{
	class ComponentRegistry
	{
	public:
		using TCreateMethod = Ref<IEntityComponent>(*)();

	public:
		ComponentRegistry() = delete;

		static bool Register(const std::string& name, TCreateMethod func);
		static Ref<IEntityComponent> Create(const std::string& name);

	private:
		static std::unordered_map<std::string, TCreateMethod>& s_Methods();
	};
}