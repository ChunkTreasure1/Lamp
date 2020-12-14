#include "lppch.h"
#include "Matrix.h"

namespace Lamp::Math
{
#ifdef LP_DX11
	namespace dx = DirectX;

	typedef dx::XMMATRIX mat4;

	dx::XMMATRIX translate(const glm::vec3& v)
	{
		return dx::XMMatrixTranslation(v.x, v.y, v.z);
	}

	dx::XMMATRIX rotateX(float angle)
	{
		return dx::XMMatrixRotationX(angle);
	}

	dx::XMMATRIX rotateY(float angle)
	{
		return dx::XMMatrixRotationY(angle);
	}

	dx::XMMATRIX rotateZ(float angle)
	{
		return dx::XMMatrixRotationZ(angle);
	}

	dx::XMMATRIX scale(const glm::vec3& scale)
	{
		return dx::XMMatrixScaling(scale.x, scale.y, scale.z);
	}

	dx::XMMATRIX perspective(float fov, float aspect, float n, float f)
	{
		return dx::XMMatrixPerspectiveFovLH(fov, aspect, n, f);
	}

	dx::XMMATRIX buildMatrix(const glm::vec3& pos, const glm::vec3& s, const glm::vec3& rot)
	{
		return rotateX(rot.x) * rotateY(rot.y) * rotateZ(rot.z) * scale(s) * translate(pos);
	}

	void* valuePtr(mat4& mat)
	{
		return &mat.r[0];
	}

#elif LP_OPENGL

	typedef glm::mat4 mat4;

	glm::mat4 translate(const glm::vec3& v)
	{
		return glm::translate(glm::mat4(1.f), v);
	}

	glm::mat4 rotateX(float angle)
	{
		return glm::rotate(glm::mat4(1.f), angle, { 1.f, 0.f, 0.f });
	}

	glm::mat4 rotateY(float angle)
	{
		return glm::rotate(glm::mat4(1.f), angle, { 0.f, 1.f, 0.f });
	}

	glm::mat4 rotateZ(float angle)
	{
		return glm::rotate(glm::mat4(1.f), angle, { 0.f, 0.f, 1.f });
	}

	glm::mat4 scale(const glm::vec3& s)
	{
		return glm::scale(glm::mat4(1.f), s);
	}

	glm::mat4 perspective(float fov, float aspect, float n, float f)
	{
		return glm::perspective(fov, aspect, n, f);
	}

	glm::mat4 buildMatrix(const glm::vec3& pos, const glm::vec3& s, const glm::vec3& rot)
	{
		return translate(pos) * scale(s) * rotateX(rot.x) * rotateY(rot.y) * rotateZ(rot.z);
	}

	void* valuePtr(mat4& mat)
	{
		return glm::value_ptr(mat);
	}

#endif
}