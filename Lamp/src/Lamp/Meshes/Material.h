#pragma once
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class Material
	{
	public:
		Material(Ref<Texture2D> diff, Ref<Texture2D> spec, Ref<Shader> shader, uint32_t matIndex)
			: m_pDiffuse(diff), m_pSpecular(spec), m_pShader(shader), m_Shininess(32.f), m_Index(matIndex)
		{}

		Material(uint32_t index)
			: m_Index(index), m_Shininess(32.f)
		{}
		//Setting
		inline void SetDiffuse(Ref<Texture2D> diff) { m_pDiffuse = diff; }
		inline void SetSpecular(Ref<Texture2D> spec) { m_pSpecular = spec; }
		inline void SetShader(Ref<Shader> shader) { m_pShader = shader; }
		inline void SetShininess(float val) { m_Shininess = val; }
		inline void SetName(const std::string& name) { m_Name = name; }

		//Getting
		inline const Ref<Texture2D>& GetDiffuse() { return m_pDiffuse; }
		inline const Ref<Texture2D>& GetSpecular() { return m_pSpecular; }
		inline const uint32_t GetIndex() { return m_Index; }
		inline const Ref<Shader>& GetShader() { return m_pShader; }
		inline const std::string& GetName() { return m_Name; }
		inline const float GetShininess() { return m_Shininess; }

	private:
		Ref<Texture2D> m_pDiffuse;
		Ref<Texture2D> m_pSpecular;
		float m_Shininess;

		Ref<Shader> m_pShader;
		uint32_t m_Index;
		std::string m_Name;
	};
}