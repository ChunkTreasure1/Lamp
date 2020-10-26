#pragma once

namespace Sandbox3D
{
	enum class Cmd
	{
		Position,
		Rotation,
		Scale,
		Transform,
		Selection
	};

	struct Command
	{
		Cmd cmd;
		void* lastData = nullptr;
		void* object = nullptr;
	};
}