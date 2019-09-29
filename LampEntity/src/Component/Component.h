#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

/*-------------------------------------------------------------------------------
Author : Ivar Jönsson
Project : PU
Purpose :
-------------------------------------------------------------------------------*/

class CComponent final : public IEntityComponent {
public:
	CComponent() = default;
	virtual ~CComponent() {}

	// Reflect type to set a unique identifier for this component
	// and provide additional information to expose it in the sandbox
	static void ReflectType(Schematyc::CTypeDesc<CComponent>& desc) {
		desc.SetGUID(""_cry_guid);
		desc.SetEditorCategory("");
		desc.SetLabel("");
		desc.SetDescription("");
		desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach });
	}

public:

	//Functions

private:

	//Member vars
};

