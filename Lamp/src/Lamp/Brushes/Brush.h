#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class Brush
	{
	public:
		Brush(const std::string& path)
			: m_Position(0.f), m_Rotation(0.f), m_Scale(1.f), m_ModelMatrix(1.f), m_Model(path)
		{}

		void Draw()
		{
			m_Model.Draw();
		}

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; CalculateModelMatrix(); m_Model.SetModelMatrix(m_ModelMatrix); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); m_Model.SetModelMatrix(m_ModelMatrix); }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; CalculateModelMatrix(); m_Model.SetModelMatrix(m_ModelMatrix); }
		
		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }
		inline glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		inline Model& GetModel() { return m_Model; }

	private:
		void CalculateModelMatrix()
		{
			m_ModelMatrix = glm::translate(glm::mat4(1.f), m_Position)
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.x), glm::vec3(1.f, 0.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f))
				* glm::scale(glm::mat4(1.f), m_Scale);
		}

	private:
		Model m_Model;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		glm::mat4 m_ModelMatrix;
	};
}