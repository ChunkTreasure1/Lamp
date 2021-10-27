#pragma once

#include <stdint.h>
#include "Lamp/Core/UUID.h"
#include "Lamp/Core/Core.h"

#include <filesystem>

namespace Lamp
{
	enum class AssetFlag : uint16_t
	{
		None = 0,
		Missing = BIT(0),
		Invalid = BIT(1)
	};

	enum class AssetType : uint16_t
	{
		None = 0,
		Level = 1,
		Mesh = 2,
		Material = 3,
		Texture = 4,
		Audio = 5,
		EnvironmentMap = 6,
		RenderGraph = 7,
		MeshSource = 8
	};

	inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		{ ".level", AssetType::Level },
		{ ".lgf", AssetType::Mesh },
		{ ".fbx", AssetType::MeshSource },
		{ ".mtl", AssetType::Material },
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".tga", AssetType::Texture },
		{ ".wav", AssetType::Audio },
		{ ".ogg", AssetType::Audio },
		{ ".hdr", AssetType::EnvironmentMap },
		{ ".rendergraph", AssetType::RenderGraph }
	};

	using AssetHandle = UUID;

	class Asset
	{
	public:
		AssetHandle Handle;

		uint16_t Flags = (uint16_t)AssetFlag::None;
		std::filesystem::path Path;
	
		virtual ~Asset() {}

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetType() { return AssetType::None; }

		bool IsValid() const { return ((Flags & (uint16_t)AssetFlag::Missing) | (Flags & (uint16_t)AssetFlag::Invalid)) == 0; }
		virtual bool operator==(const Asset& other) const
		{
			return Handle == other.Handle;
		}

		virtual bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}

		bool IsFlagSet(AssetFlag flag) const { return (uint16_t)flag & Flags; }
		void SetFlag(AssetFlag flag, bool value = true)
		{
			if (value)
			{
				Flags |= (uint16_t)flag;
			}
			else
			{
				Flags &= ~(uint16_t)flag;
			}
		}
	};
}