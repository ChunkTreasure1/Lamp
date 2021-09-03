#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Utility/PlatformUtility.h"

namespace Sandbox3D
{
	void Sandbox3D::SaveLevelAs()
	{
		std::string filepath = Lamp::FileDialogs::SaveFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			Lamp::LevelSystem::SaveLevel(filepath, Lamp::LevelSystem::GetCurrentLevel());
		}
	}

	void Sandbox3D::OpenLevel()
	{
		std::string filepath = Lamp::FileDialogs::OpenFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			OpenLevel(filepath);
		}
	}

	void Sandbox3D::OpenLevel(const std::filesystem::path& path)
	{
		Lamp::LevelSystem::LoadLevel(path.string());
		m_pSelectedObject = nullptr;
	}

	void Sandbox3D::NewLevel()
	{
		if (!Lamp::LevelSystem::GetCurrentLevel()->GetPath().empty())
		{
			Lamp::LevelSystem::SaveLevel(Lamp::LevelSystem::GetCurrentLevel());
		}

		Ref<Lamp::Level> pLevel = CreateRef<Lamp::Level>("New Level", "");
		Lamp::LevelSystem::SetCurrentLevel(pLevel);
		m_pSelectedObject = nullptr;
	}

	void Sandbox3D::Undo()
	{
		m_ActionHandler.Undo();
	}

	void Sandbox3D::Redo()
	{
		m_ActionHandler.Redo();
	}
}