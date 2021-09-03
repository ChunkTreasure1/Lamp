#pragma once

#include <vector>
#include "Brush.h"
#include "Brush2D.h"
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
		Brush* Create(Brush* main);

		Brush2D* Create2D(const std::string& path);
		Brush2D* Create2D(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, uint32_t layerId, const std::string& name);

		void Remove(Brush* brush);
		void Remove(Brush2D* brush);

		//Setting
		inline void SetBrushes(std::unordered_map<uint32_t, Brush*> brushes) { m_Brushes = brushes; }
		inline void SetBrushes(std::vector<Brush2D*> brushes) { m_2DBrushes = brushes; }

		//Getting
		inline std::unordered_map<uint32_t, Brush*>& GetBrushes() { return m_Brushes; }
		inline std::vector<Brush2D*>& Get2DBrushes() { return m_2DBrushes; }

		Brush* GetBrushFromPoint(const glm::vec3& pos, const glm::vec3& origin);
		Brush2D* GetBrush2DFromPoint(const glm::vec3& pos, const glm::vec3& origin);

	private:
		std::unordered_map<uint32_t, Brush*> m_Brushes;
		std::vector<Brush2D*> m_2DBrushes;
	};
}