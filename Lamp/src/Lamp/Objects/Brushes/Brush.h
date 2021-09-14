#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Objects/Object.h"

namespace Lamp
{
	class Brush : public Object
	{
	public:
		Brush(Ref<Mesh> model);

		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;
		virtual uint64_t GetEventMask() override { return EventType::AppRender | EventType::AppUpdate; }

		//Getting
		inline Ref<Mesh>& GetModel() { return m_Model; }

	protected:
		virtual void ScaleChanged() override;

	public:
		static Brush* Create(const std::string& path);
		static Brush* Create(const std::string& path, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, uint32_t layerId, const std::string& name);
		static Brush* Duplicate(Brush* main, bool addToLevel);
		static Brush* Get(uint32_t id);

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);

	private:
		Ref<Mesh> m_Model;
	};
}