#pragma once

#include <vector>
#include "Brush.h"
#include "Lamp/Event/Event.h"

namespace Lamp
{
	class BrushManager
	{
	public:
		BrushManager();
		~BrushManager();

		Brush* Create(const std::string& path);
		Brush* Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, uint32_t layerId, const std::string& name);
		void Remove(Brush* brush);

		//Setting
		inline void SetBrushes(std::vector<Brush*> brushes) { m_Brushes = brushes; }

		//Getting
		inline std::vector<Brush*>& GetBrushes() { return m_Brushes; }
		Brush* GetBrushFromPoint(const glm::vec3& pos, const glm::vec3& origin);
		Brush* GetBrushFromPhysicalEntity(Ref<PhysicalEntity>& pEnt);

	public:
		static void SetCurrentManager(Ref<BrushManager> manager) { s_Manager = manager; }
		static Ref<BrushManager>& Get() { return s_Manager; }

	private:
		std::vector<Brush*> m_Brushes;

	private:
		static Ref<BrushManager> s_Manager;
	};
}