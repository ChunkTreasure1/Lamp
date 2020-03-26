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

		void Draw();
		Brush* Create(const std::string& path);
		Brush* Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		void Remove(const Brush* brush);
		void OnEvent(Event& e);

		Brush* GetBrushFromPoint(const glm::vec2& pos);

		//Setting
		inline void SetBrushes(std::vector<Brush*> brushes) { m_Brushes = brushes; }

		//Getting
		inline std::vector<Brush*>& GetBrushes() { return m_Brushes; }

	public:
		static void SetCurrentManager(Ref<BrushManager> manager) { s_Manager = manager; }
		static Ref<BrushManager>& Get() { return s_Manager; }

	private:
		std::vector<Brush*> m_Brushes;

	private:
		static Ref<BrushManager> s_Manager;
	};
}