#pragma once
#include <cstdint>
#include <vector>
#include <iostream>

namespace Asset
{
	enum class CompressionMode : uint8_t
	{
		None,
		LZ4
	};

	struct Buffer
	{
	public:
		Buffer();

		void CopyFrom(const void* src, size_t size, CompressionMode compressionMode);
		void CopyTo(void* dst) const;

		size_t TotalBufferSize() const;

		friend std::ostream& operator<<(std::ostream& os, const Buffer& buffer);
		friend std::istream& operator>>(std::istream& os, Buffer& buffer);
	private:
		std::vector<uint8_t> m_buffer;
		CompressionMode m_compressionMode;
		size_t m_totalBufferSize;
		size_t m_compressedBufferSize;
	};

	std::ostream& operator<<(std::ostream& os, const Buffer& buffer);
	std::istream& operator>>(std::istream& os, Buffer& buffer);
}