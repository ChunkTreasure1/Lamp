#pragma once

namespace Lamp
{
	class Spline
	{
	public:
		virtual ~Spline() = default;
		void AddPoint(const glm::vec3& point);

	protected:
		std::vector<glm::vec3> m_points;
	};

	class TerrainSpline : public Spline
	{
	public:
		TerrainSpline();
		~TerrainSpline() override;

	private:
		float m_width;
	};
}