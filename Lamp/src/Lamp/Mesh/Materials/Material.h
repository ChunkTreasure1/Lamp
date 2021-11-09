#pragma once
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include <unordered_map>
#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Material : public Asset
	{
	public:
		Material(Ref<Shader> shader, uint32_t id)
			: m_Index(id)
		{
			SetShader(shader);
		}

		Material()
			: m_Index(0)
		{}

		Material(uint32_t index, const std::string& name)
			: m_Index(index), m_Name(name)
		{}
		//Setting
		inline void SetTextures(std::unordered_map<std::string, Ref<Texture2D>>& map) { m_pTextures = map; }
		void SetTexture(const std::string& name, Ref<Texture2D> texture);
		void SetShader(Ref<Shader> shader);
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetBlendingMutliplier(float value) { m_blendingMultiplier = value; }
		inline void SetUseBlending(bool state) { m_useBlending = state; }

		//Getting
		inline std::unordered_map<std::string, Ref<Texture2D>>& GetTextures() { return m_pTextures; }
		inline const uint32_t GetIndex() { return m_Index; }
		inline const Ref<Shader>& GetShader() { return m_pShader; }
		inline const std::string& GetName() { return m_Name; }
		inline const float& GetBlendingMultiplier() { return m_blendingMultiplier; }
		inline const bool& GetUseBlending() { return m_useBlending; }

		static AssetType GetStaticType() { return AssetType::Material; }
		virtual AssetType GetType() override { return GetStaticType(); }

	private:
		std::unordered_map<std::string, Ref<Texture2D>> m_pTextures;

		Ref<Shader> m_pShader;
		uint32_t m_Index;
		std::string m_Name;
		float m_blendingMultiplier = 1.f;
		bool m_useBlending = false;
	};
}