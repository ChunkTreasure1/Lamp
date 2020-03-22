#pragma once

#include <vector>
#include "Brush2D.h"

namespace Lamp
{
	class BrushManager
	{
	public:
		BrushManager();
		~BrushManager();

		void Draw();
		Brush2D* Create(const std::string& spritePath);
		Brush2D* Create(const std::string& spriteBath, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, bool shouldCollide);
		void Remove(const Brush2D* brush);

		Brush2D* GetBrushFromPoint(const glm::vec2& pos);

		//Setting
		inline void SetBrushes(std::vector<Brush2D*> brushes) { m_Brushes = brushes; }

		//Getting
		inline std::vector<Brush2D*>& GetBrushes() { return m_Brushes; }

	public:
		static void SetCurrentManager(BrushManager& manager) { s_Manager = manager; }
		static BrushManager& Get() { return s_Manager; }

	private:
		std::vector<Brush2D*> m_Brushes;

	private:
		static BrushManager s_Manager;
	};
}