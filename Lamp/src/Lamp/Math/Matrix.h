#pragma once

#include <glm/glm.hpp>
#include <DirectXMath.h>

namespace Lamp::Math
{
#ifdef LP_DX11
	namespace dx = DirectX;

	typedef dx::XMMATRIX mat4;

	dx::XMMATRIX translate(const glm::vec3& v);
	dx::XMMATRIX rotateX(float angle);
	dx::XMMATRIX rotateY(float angle);
	dx::XMMATRIX rotateZ(float angle);
	dx::XMMATRIX scale(const glm::vec3& scale);
	dx::XMMATRIX perspective(float fov, float aspect, float n, float f);
	dx::XMMATRIX transpose(const dx::XMMATRIX& mat);
	dx::XMMATRIX buildMatrix(const glm::vec3& pos, const glm::vec3& s, const glm::vec3& rot);

	void* valuePtr(mat4& mat);

#elif LP_OPENGL

	typedef glm::mat4 mat4;

	glm::mat4 translate(const glm::vec3& v);
	glm::mat4 rotateX(float angle);
	glm::mat4 rotateY(float angle);
	glm::mat4 rotateZ(float angle);
	glm::mat4 scale(const glm::vec3& s);
	glm::mat4 perspective(float fov, float aspect, float n, float f);
	glm::mat4 buildMatrix(const glm::vec3& pos, const glm::vec3& s, const glm::vec3& rot);
	void* valuePtr(mat4& mat);
#endif
}