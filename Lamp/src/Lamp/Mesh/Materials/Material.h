#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include <unordered_map>

namespace Lamp
{
	class Texture2D;
	class Shader;
	class RenderPipeline;
	
	struct MaterialData
	{
		bool useBlending = false;
		bool useTranslucency = false;
		bool useAlbedo = true;
		bool useNormal = true;
		bool useMRO = true;
		bool useDetailNormal = false;

		float blendingMultiplier = 0.f;

		glm::vec2 mroColor{ 0.f, 1.f };
		glm::vec3 normalColor{ 0.f, 1.f, 0.f };
		glm::vec4 albedoColor{ 1.f, 1.f, 1.f, 1.f };
	};
	
	class Material : public Asset
	{
	public:
		virtual ~Material() {}
		virtual void Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex = 0) = 0;

		//Setting
		virtual void SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& map) = 0;
		virtual void SetTexture(const std::string& name, Ref<Texture2D> texture) = 0;
		virtual void SetShader(Ref<Shader> shader) = 0;
		virtual void SetName(const std::string& name) = 0;

		//Getting
		virtual const std::vector<Ref<Texture2D>> GetTextures() = 0;
		virtual const uint32_t GetIndex() = 0;
		virtual Ref<Shader> GetShader() = 0;
		virtual const std::string& GetName() = 0;
		virtual const MaterialData& GetMaterialData() const = 0;

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Material> Create(Ref<Shader> shader, uint32_t id);
		static Ref<Material> Create();
		static Ref<Material> Create(const Ref<Material> material);
		static Ref<Material> Create(const std::string& name, uint32_t index);
	};
}