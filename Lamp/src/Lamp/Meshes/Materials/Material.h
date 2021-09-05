#pragma once
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include <unordered_map>

namespace Lamp
{
	class Material
	{
	public:
		Material(Ref<Shader> shader, uint32_t id)
			: m_Index(id)
		{
			SetShader(shader);
		}

		Material()
		{}

		Material(uint32_t index)
			: m_Index(index), m_Shininess(32.f)
		{}
		//Setting
		inline void SetTextures(std::unordered_map<std::string, Ref<Texture2D>>& map) { m_pTextures = map; }
		void SetTexture(const std::string& name, Ref<Texture2D>& texture);
		void SetShader(Ref<Shader> shader);
		inline void SetShininess(float val) { m_Shininess = val; }
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetPath(const std::string& path) { m_Path = path; }

		//Getting
		inline std::unordered_map<std::string, Ref<Texture2D>>& GetTextures() { return m_pTextures; }
		inline const uint32_t GetIndex() { return m_Index; }
		inline const Ref<Shader>& GetShader() { return m_pShader; }
		inline std::string& GetName() { return m_Name; }
		inline const float GetShininess() { return m_Shininess; }
		inline const std::string& GetPath() { return m_Path; }

		void UploadData();

	private:
		std::unordered_map<std::string, Ref<Texture2D>> m_pTextures;
		float m_Shininess;
		float m_DepthScale = 0.1f;

		Ref<Shader> m_pShader;
		uint32_t m_Index;
		std::string m_Name;
		std::string m_Path;
	};
}