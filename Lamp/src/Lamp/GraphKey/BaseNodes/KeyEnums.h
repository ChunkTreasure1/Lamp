#pragma once

#include <map>
#include <string>

namespace Lamp
{
	enum class KeyEnum
	{
		kSpace = 32,
		k0 = 48,
		k1 = 49,
		k2 = 50,
		k3 = 51,
		k4 = 52,
		k5 = 53,
		k6 = 54,
		k7 = 55,
		k8 = 56,
		k9 = 57,

		kA = 65,
		kB = 66,
		kC = 67,
		kD = 68,
		kE = 69,
		kF = 70,
		kG = 71,
		kH = 72,
		kI = 73,
		kJ = 74,
		kK = 75,
		kL = 76,
		kM = 77,
		kN = 78,
		kO = 79,
		kP = 80,
		kQ = 81,
		kR = 82,
		kS = 83,
		kT = 84,
		kU = 85,
		kV = 86,
		kW = 87,
		kX = 88,
		kY = 89,
		kZ = 90,
		kWorld1 = 161,
		kWorld2 = 162,
		
		kEscape = 256,
		kEnter = 257,
		kTab = 258,
		kBackspace = 259,
		kInsert = 260,
		kDelete = 261,
		kRight = 262,
		kLeft = 263,
		kDown = 264,
		kUp = 265,

		kLeftShift = 340,
		kLeftControl = 341,
		kLeftAlt = 342,
		kLeftSuper = 343,
		kRightShift = 344,
		kRightControl = 345,
		kRightAlt = 346,
		kRightSuper = 347
	};

	static std::map<std::string, int> GetMap()
	{
		static std::map<std::string, int> impl =
		{
			{ "Space", (int)KeyEnum::kSpace },
			{ "0", (int)KeyEnum::k0 },
			{ "1", (int)KeyEnum::k1 },
			{ "2", (int)KeyEnum::k2 },
			{ "3", (int)KeyEnum::k3 },
			{ "4", (int)KeyEnum::k4 },
			{ "5", (int)KeyEnum::k5 },
			{ "6", (int)KeyEnum::k6 },
			{ "7", (int)KeyEnum::k7 },
			{ "8", (int)KeyEnum::k8 },
			{ "9", (int)KeyEnum::k9 },

			{ "A", (int)KeyEnum::kA },
			{ "B", (int)KeyEnum::kB },
			{ "C", (int)KeyEnum::kC },
			{ "D", (int)KeyEnum::kD },
			{ "E", (int)KeyEnum::kE },
			{ "F", (int)KeyEnum::kF },
			{ "G", (int)KeyEnum::kG },
			{ "H", (int)KeyEnum::kH },
			{ "I", (int)KeyEnum::kI },
			{ "J", (int)KeyEnum::kJ },
			{ "K", (int)KeyEnum::kK },
			{ "L", (int)KeyEnum::kL },
			{ "M", (int)KeyEnum::kM },
			{ "N", (int)KeyEnum::kN },
			{ "O", (int)KeyEnum::kO },
			{ "P", (int)KeyEnum::kP },
			{ "Q", (int)KeyEnum::kQ },
			{ "R", (int)KeyEnum::kR },
			{ "S", (int)KeyEnum::kS },
			{ "T", (int)KeyEnum::kT },
			{ "U", (int)KeyEnum::kU },
			{ "V", (int)KeyEnum::kV },
			{ "W", (int)KeyEnum::kW },
			{ "X", (int)KeyEnum::kX },
			{ "Y", (int)KeyEnum::kY },
			{ "Z", (int)KeyEnum::kZ },
			{ "World1", (int)KeyEnum::kWorld1 },
			{ "World2", (int)KeyEnum::kWorld2 },

			{ "Escape", (int)KeyEnum::kEscape },
			{ "Enter", (int)KeyEnum::kEnter },
			{ "Tab", (int)KeyEnum::kTab },
			{ "Backspace", (int)KeyEnum::kBackspace },
			{ "Insert", (int)KeyEnum::kInsert },
			{ "Delete", (int)KeyEnum::kDelete },
			{ "Right", (int)KeyEnum::kRight },
			{ "Left", (int)KeyEnum::kLeft },
			{ "Down", (int)KeyEnum::kDown },
			{ "Up", (int)KeyEnum::kUp },

			{ "LeftShift", (int)KeyEnum::kLeftShift },
			{ "LeftControl", (int)KeyEnum::kLeftControl },
			{ "LeftAlt", (int)KeyEnum::kLeftAlt },
			{ "LeftSuper", (int)KeyEnum::kLeftSuper },
			{ "RightShift", (int)KeyEnum::kRightShift },
			{ "RightControl", (int)KeyEnum::kRightControl },
			{ "RightAlt", (int)KeyEnum::kRightAlt },
			{ "RightSuper", (int)KeyEnum::kRightSuper }
		};

		return impl;
	}
}