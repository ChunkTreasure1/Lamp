#pragma once

#include "Asset.h"

namespace Lamp
{
	class AssetLoader
	{
	public:
		virtual void Save(const Ref<Asset>& asset) const = 0;
		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const = 0;
	};

	class MeshLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override;
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};

	class TextureLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override;
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};

	class EnvironmentLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override {}
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};

	class MaterialLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override;
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};

	class MeshSourceLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override;
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};
}