#pragma once

#include <stdint.h>
#include <xhash>

namespace Lamp
{
	class UUID
	{
	public:
		UUID();
		UUID(uint32_t uuid);
		UUID(const UUID& other);

		operator uint32_t() { return m_UUID; }
		operator const uint32_t() const { return m_UUID; }

	private:
		uint32_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Lamp::UUID>
	{
		std::size_t operator()(const Lamp::UUID& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}
	};
}