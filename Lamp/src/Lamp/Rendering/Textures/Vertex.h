#pragma once

#include <GL/glew.h>

namespace Lamp
{
	struct UV
	{
		float U;
		float V;
	};

	//Struct for positions
	struct Vec2
	{
		float X;
		float Y;
	};

	//Struct for colors
	struct Color
	{
		GLubyte R;
		GLubyte G;
		GLubyte B;

		GLubyte A;
	};

	struct Vertex
	{
		Vec2 Position;
		Color Color;
		UV UV;

		void SetColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
		{
			Color.R = r;
			Color.G = g;
			Color.B = b;

			Color.A = a;
		}

		void SetUV(float u, float v)
		{
			UV.U = u;
			UV.V = v;
		}

		void SetPos(float x, float y)
		{
			Position.X = x;
			Position.Y = y;
		}
	};
}