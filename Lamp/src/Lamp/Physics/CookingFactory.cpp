#include "lppch.h"
#include "CookingFactory.h"
#include "PhysXInternal.h"

#include "Lamp/Core/Buffer.h"

namespace Lamp
{
    struct CookingData
    {
        physx::PxCooking* CookingSDK;
        physx::PxCookingParams CookingParams;

        CookingData(const physx::PxTolerancesScale& scale)
            : CookingSDK(nullptr), CookingParams(scale)
        {}
    };

    static CookingData* s_CookingData = nullptr;

    void CookingFactory::Initialize()
    {
        s_CookingData = new CookingData(PhysXInternal::GetPhysXSDK().getTolerancesScale());
        s_CookingData->CookingParams.meshWeldTolerance = 0.1f;
        s_CookingData->CookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
        s_CookingData->CookingParams.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

        s_CookingData->CookingSDK = PxCreateCooking(PX_PHYSICS_VERSION, PhysXInternal::GetFoundation(), s_CookingData->CookingParams);
        LP_CORE_ASSERT(s_CookingData->CookingSDK, "Couldn't initialize PhysX Cooking SDK");
    }

    void CookingFactory::Shutdown()
    {
        s_CookingData->CookingSDK->release();
        s_CookingData->CookingSDK = nullptr;

        delete s_CookingData;
    }
   
    namespace Utils
    {
        static std::filesystem::path GetCacheDirectory()
        {
            return std::filesystem::path("Cache") / std::filesystem::path("Colliders");
        }

		static void CreateCacheDirectoryIfNeeded()
		{
            std::filesystem::path cacheDirectory = GetCacheDirectory();
            if (!std::filesystem::exists(cacheDirectory))
            {
                std::filesystem::create_directories(cacheDirectory);
            }
		}
    }


    CookingResult CookingFactory::CookMesh(Ref<MeshColliderComponent> component, bool invalidateOld, std::vector<MeshColliderData>& outData)
    {
        Utils::CreateCacheDirectoryIfNeeded();

        auto& collisionMesh = component->GetSpecification().CollisionMesh;
        if (!collisionMesh->IsValid())
        {
            LP_CORE_ERROR("Invalid mesh!");
            return CookingResult::Failure;
        }

        CookingResult result = CookingResult::Failure;
        std::filesystem::path filepath = Utils::GetCacheDirectory() / (collisionMesh->Path.stem().string() + (component->GetSpecification().IsConvex ? "_convex.pxm" : "_tri.pxm"));

        if (invalidateOld)
        {
            component->GetSpecification().ProcessedMeshes.clear();
            bool removedCached = std::filesystem::remove(filepath);
            if (!removedCached)
            {
                LP_CORE_WARN("Could not delete cached collider data for {0}", filepath);
            }
        }

        if (!std::filesystem::exists(filepath))
        {
            result = component->GetSpecification().IsConvex ? CookConvexMesh(collisionMesh, outData) : CookTriangleMesh(collisionMesh, outData);
            if (result == CookingResult::Success)
            {
                //Serialize collider
                uint32_t bufferSize = 0;
                uint32_t offset = 0;

                for (auto& colliderData : outData)
                {
                    bufferSize += sizeof(uint32_t);
                    bufferSize += colliderData.Size;
                }

                Buffer colliderBuffer;
                colliderBuffer.Allocate(bufferSize);

                for (auto& colliderData : outData)
                {
                    colliderBuffer.Write((void*)&colliderData.Size, sizeof(uint32_t), offset);
                    offset += sizeof(uint32_t);
                    colliderBuffer.Write(colliderData.Data, colliderData.Size, offset);
                    offset += colliderData.Size;
                }

                bool success = FileSystem::WriteBytes(filepath, colliderBuffer);
                colliderBuffer.Release();

                if (!success)
                {
                    LP_CORE_ERROR("Failed to write collider to {0}", filepath.string());
                    return CookingResult::Failure;
                }
            }
        }
        else
        {
            Buffer colliderBuffer = FileSystem::ReadBytes(filepath);

            if (colliderBuffer.Size > 0)
            {
                uint32_t offset = 0;

                const auto& submeshes = collisionMesh->GetSubMeshes();

                for (const auto& submesh : submeshes)
                {
                    MeshColliderData& data = outData.emplace_back();
                    
                    data.Size = colliderBuffer.Read<uint32_t>(offset);
                    offset += sizeof(uint32_t);

                    data.Data = colliderBuffer.ReadBytes(data.Size, offset);
                    offset += data.Size;
                    data.Transform = glm::mat4(0.f);
                }

                colliderBuffer.Release();

                result = CookingResult::Success;
            }
        }

        if (result == CookingResult::Success && component->GetSpecification().ProcessedMeshes.size() == 0)
        {
            for (const auto& colliderData : outData)
            {
                GenerateDebugMesh(component, colliderData);
            }
        }

        return result;
    }
    
    CookingResult CookingFactory::CookConvexMesh(const Ref<Mesh>& mesh, std::vector<MeshColliderData>& outData)
    {
        return CookingResult::Failure;
    }
    
    CookingResult CookingFactory::CookTriangleMesh(const Ref<Mesh>& mesh, std::vector<MeshColliderData>& outData)
    {
        return CookingResult();
    }

	void CookingFactory::GenerateDebugMesh(Ref<MeshColliderComponent> component, const MeshColliderData& colliderData)
	{
	}
}