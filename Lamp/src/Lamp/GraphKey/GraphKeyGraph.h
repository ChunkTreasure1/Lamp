#pragma once

#include "Node.h"

namespace Lamp
{
	struct GraphKeyGraphSpecification
	{
		std::vector<Node> nodes;
		std::string name;
		std::string path;
	};

	class GraphKeyGraph
	{
	public:
		GraphKeyGraph();
		GraphKeyGraph(const GraphKeyGraphSpecification& spec);

		inline GraphKeyGraphSpecification& GetSpecification() { return m_Specification; }

	private:
		GraphKeyGraphSpecification m_Specification;
	};
}