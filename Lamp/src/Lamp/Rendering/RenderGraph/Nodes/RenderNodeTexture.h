#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class Texture2D;
	class RenderNodeTexture : public RenderNode
	{
	public:
		RenderNodeTexture()
		{
		}

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Texture; }
		virtual void Serialize(YAML::Emitter& out) override;
		virtual void Deserialize(YAML::Node& node) override;

		Ref<Texture2D> texture;

	private:
		void GetTexture();

	private:
		bool m_UseInternalTextures = false;
		int m_CurrentlySelectedTexture = 0;
		std::string m_SelectedTextureName = "";
		std::vector<const char *> m_TextureNames;
	};
}