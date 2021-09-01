#pragma once

namespace Lamp
{
	enum class Type
	{
		Int,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		IntArray
	};

	static uint32_t GetSizeFromType(Type elementType)
	{
		switch (elementType)
		{
		case Type::Bool: return 1;
		case Type::Int: return 4;
		case Type::Float: return 4;
		case Type::Float2: return 4 * 2;
		case Type::Float3: return 4 * 3;
		case Type::Float4: return 4 * 4;
		case Type::Mat3: return 4 * 3 * 3;
		case Type::Mat4: return 4 * 4 * 4;
		}

		return 0;
	}
}