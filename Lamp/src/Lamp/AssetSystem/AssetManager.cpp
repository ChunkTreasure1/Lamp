#include "lppch.h"
#include "AssetManager.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Texture2D/TextureCache.h"

#include "Lamp/Meshes/ModelLoader.h"

namespace Lamp
{
	AssetManager::AssetManager()
	{
		Initialize();
	}

	AssetManager::~AssetManager()
	{
		Shutdown();
	}

	void AssetManager::Initialize()
	{
		for (uint32_t i = 0; i < m_MaxThreads; i++)
		{
			m_WorkerThreads.push_back(std::thread(&AssetManager::LoaderThread, this));
		}

		m_ThreadNames.emplace(std::this_thread::get_id(), "Main");

		for (uint32_t i = 0; i < m_MaxThreads; i++)
		{
			std::string name = "T" + std::to_string(i);
			m_ThreadNames.emplace(m_WorkerThreads[i].get_id(), name);
		}
	}

	void AssetManager::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		m_LoadingThreadActive = false;
		for (uint32_t i = 0; i < m_MaxThreads; i++)
		{
			m_WorkerThreads[i].join();
		}
	}

	void AssetManager::LoaderThread()
	{
		{
			std::scoped_lock<std::mutex> lock(m_OutputMutex);
			LP_PROFILE_FUNCTION();
		}

		while (m_LoadingThreadActive)
		{
			if (!m_LoadingTexturesQueue.Empty())
			{
				TextureLoadJob assetJob;
				if (m_LoadingTexturesQueue.TryPop(assetJob))
				{
					TextureLoader::LoadTexture(assetJob.data, assetJob.path);
					m_ProcessingTexturesQueue.Push(assetJob);
				}
			}

			if (!m_LoadingModelsQueue.Empty())
			{
				ModelLoadJob modelJob;
				if (m_LoadingModelsQueue.TryPop(modelJob))
				{
					ModelLoader::LoadFromFile(modelJob.data, modelJob.path);
					m_ProcessingModelsQueue.Push(modelJob);
				}
			}

			uint32_t workRemaining = m_LoadingTexturesQueue.Size();
			const int waitTime = workRemaining > 0 ? 10 : 2000;
			std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
		}
	}

	void AssetManager::Update()
	{
		if (!m_ProcessingTexturesQueue.Empty())
		{
			TextureLoadJob assetJob;
			if (m_ProcessingTexturesQueue.TryPop(assetJob))
			{
				if (assetJob.pTexture)
				{
					TextureData t;
					if (assetJob.data.type == TextureType::Texture2D)
					{
						t = TextureLoader::GenerateTexture(assetJob.data);
					}
					else if (assetJob.data.type == TextureType::HDR)
					{
						t = TextureLoader::GenerateHDR(assetJob.data);
					}
					TextureCache::AddTexture(assetJob.path, t);
					assetJob.pTexture->SetData(t);
					assetJob.pTexture->SetType(assetJob.data.type);
				}
			}
		}

		if (!m_ProcessingModelsQueue.Empty())
		{
			ModelLoadJob modelJob;
			if (m_ProcessingModelsQueue.TryPop(modelJob))
			{
				if (modelJob.pModel)
				{
					ModelData d = ModelLoader::GenerateMesh(modelJob.data);
					modelJob.pModel->SetData(d);
				}
			}
		}
	}

	void AssetManager::LoadTexture(const std::string& path, Texture2D* pTex)
	{
		if (!pTex)
		{
			return;
		}

		TextureLoadJob job;
		job.pTexture = pTex;
		job.path = path;

		m_LoadingTexturesQueue.Push(job);
	}

	void AssetManager::LoadModel(const std::string& path, Model* pModel)
	{
		if (!pModel)
		{
			return;
		}

		ModelLoadJob job;
		job.pModel = pModel;
		job.path = path;

		m_LoadingModelsQueue.Push(job);
	}
}