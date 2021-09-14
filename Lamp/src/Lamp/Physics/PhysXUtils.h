#pragma once

#include <PhysX/PxPhysicsAPI.h>
#include "Lamp/Core/Core.h"
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	enum class CookingResult
	{
		Success,
		ZeroAreaTestFailed,
		PolygonLimitReached,
		LargeTriangle,
		Failure
	};

	enum class ForceMode : uint8_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class ActorLockFlag
	{
		TranslationX = BIT(0), TranslationY = BIT(1), TranslationZ = BIT(2), Translation = TranslationX | TranslationY | TranslationZ,
		RotationX = BIT(3), RotationY = BIT(4), RotationZ = BIT(5), Rotation = RotationX | RotationY | RotationZ
	};

	namespace PhysXUtils
	{
		physx::PxTransform ToPhysXTransform(const glm::mat4& transform);
		physx::PxTransform ToPhysXTransform(const glm::vec3& position, const glm::vec3& rotation);
		physx::PxMat44 ToPhysXMatrix(const glm::mat4& mat);

		const physx::PxVec3& ToPhysXVector(const glm::vec3& vec);
		const physx::PxVec4& ToPhysXVector(const glm::vec4& vec);

		physx::PxQuat ToPhysXQuat(const glm::quat& quat);

		glm::mat4 FromPhysXTransform(const physx::PxTransform& transform);
		glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix);
		glm::vec3 FromPhysXVector(const physx::PxVec3& vector);
		glm::vec4 FromPhysXVector(const physx::PxVec4& vector);
		glm::quat FromPhysXQuat(const physx::PxQuat& quat);

		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult);
		CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult);
	}
}