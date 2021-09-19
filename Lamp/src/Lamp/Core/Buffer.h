#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Core/Log.h"

namespace Lamp
{
	struct Buffer
	{
		void* pData;
		uint32_t Size;

		Buffer()
			: pData(nullptr), Size(0)
		{}

		Buffer(void* data, uint32_t size)
			: pData(data), Size(size)
		{}

		static Buffer Copy(const void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.pData, data, size);
			return buffer;
		}

		void Allocate(uint32_t size)
		{
			delete[] pData;
			pData = nullptr;

			if (size == 0)
			{
				return;
			}

			pData = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] pData;
			pData = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (pData)
			{
				memset(pData, 0, Size);
			}
		}

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)((uint8_t*)pData + offset);
		}

		uint8_t* ReadBytes(uint32_t size, uint32_t offset)
		{
			LP_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, (uint8_t*)pData + offset, size);
			return buffer;
		}

		void Write(void* data, uint32_t size, uint32_t offset = 0)
		{
			LP_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((uint8_t*)pData + offset, data, size);
		}

		operator bool() const
		{
			return pData;
		}

		uint8_t& operator[](int index)
		{
			return ((uint8_t*)pData)[index];
		}

		uint8_t operator[](int index) const
		{
			return ((uint8_t*)pData)[index];
		}

		template<typename T>
		T* As()
		{
			return (T*)pData;
		}

		inline uint32_t GetSize() const { return Size; }
	};
}