#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include <unordered_map>

namespace Lamp
{
	class Texture2D;
	class Shader;
	class RenderPipeline;
	class Material : public Asset
	{
	public:
		virtual void Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex = 0) = 0;

		//Setting
		virtual void SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& map) = 0;
		virtual void SetTexture(const std::string& name, Ref<Texture2D> texture) = 0;
		virtual void SetShader(Ref<Shader> shader) = 0;
		virtual void SetName(const std::string& name) = 0;
		virtual void SetBlendingMutliplier(float value) = 0;
		virtual void SetUseBlending(bool state) = 0;

		//Getting
		virtual const std::vector<Ref<Texture2D>> GetTextures() = 0;
		virtual const uint32_t GetIndex() = 0;
		virtual Ref<Shader> GetShader() = 0;
		virtual const std::string& GetName() = 0;
		virtual const float& GetBlendingMultiplier() = 0;
		virtual const bool& GetUseBlending() = 0;

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Material> Create(Ref<Shader> shader, uint32_t id);
		static Ref<Material> Create();
		static Ref<Material> Create(const std::string& name, uint32_t index);
	};
}