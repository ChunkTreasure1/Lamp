#pragma once

#include <stdint.h>
#include <xhash>

namespace Lamp
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID& other);

		operator uint64_t() { return m_UUID; }
		operator const uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};

	class GraphUUID
	{
	public:
		GraphUUID();
		GraphUUID(int uuid);
		GraphUUID(const GraphUUID& other);

operator int() { return m_UUID; }
		operator const int() const { return m_UUID; }

	private:
		int m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Lamp::UUID>
	{
		std::size_t operator()(const Lamp::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};

	template<>
	struct hash<Lamp::GraphUUID>
	{
		std::size_t operator()(const Lamp::GraphUUID& uuid) const
		{
			return hash<int>()((int)uuid);
		}
	};
}