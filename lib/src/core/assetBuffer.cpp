#include "core/assetBuffer.h"
#include "lz4.H"

using namespace Asset;

Buffer::Buffer() : m_compressedBufferSize(0), m_totalBufferSize(0)
{

}

void Buffer::CopyFrom(const void* src, size_t size, CompressionMode compressionMode)
{
	m_buffer.resize(size);
	m_totalBufferSize = size;
	m_compressionMode = compressionMode;

	switch (m_compressionMode)
	{
	case CompressionMode::None:
	{
		memcpy(m_buffer.data(), src, size);
		break;
	}
	case CompressionMode::LZ4:
	{
		int compressStaging = LZ4_compressBound((int)size);
		m_buffer.resize(compressStaging);
		int compressedSize = LZ4_compress_default((const char*)src, (char*)m_buffer.data(), (int)size, compressStaging);
		m_buffer.resize(compressedSize);
		m_compressedBufferSize = compressedSize;
		break;
	}
	}
}

void Buffer::CopyTo(void* dst) const
{
	switch (m_compressionMode)
	{
	case CompressionMode::None:
	{
		memcpy(dst, m_buffer.data(), m_totalBufferSize);
		break;
	}
	case CompressionMode::LZ4:
	{
		LZ4_decompress_safe((const char*)m_buffer.data(), (char*)dst, (int)m_compressedBufferSize, (int)m_totalBufferSize);
		break;
	}
	}
}

std::size_t Buffer::TotalBufferSize() const
{
	return m_totalBufferSize;
}

namespace Asset
{
	std::ostream& operator<<(std::ostream& os, const Buffer& buffer)
	{
		os.write(reinterpret_cast<const char*>(&buffer.m_compressionMode), sizeof(buffer.m_compressionMode));
		os.write(reinterpret_cast<const char*>(&buffer.m_totalBufferSize), sizeof(buffer.m_totalBufferSize));
		os.write(reinterpret_cast<const char*>(&buffer.m_compressedBufferSize), sizeof(buffer.m_compressedBufferSize));

		os.write(reinterpret_cast<const char*>(buffer.m_buffer.data()), buffer.m_buffer.size());

		return os;
	}

	std::istream& operator>>(std::istream& is, Buffer& buffer)
	{
		is.read(reinterpret_cast<char*>(&buffer.m_compressionMode), sizeof(buffer.m_compressionMode));
		is.read(reinterpret_cast<char*>(&buffer.m_totalBufferSize), sizeof(buffer.m_totalBufferSize));
		is.read(reinterpret_cast<char*>(&buffer.m_compressedBufferSize), sizeof(buffer.m_compressedBufferSize));

		size_t bufferSize = buffer.m_compressionMode != CompressionMode::None ?
			buffer.m_compressedBufferSize : buffer.m_totalBufferSize;

		buffer.m_buffer.resize(bufferSize);
		is.read(reinterpret_cast<char*>(buffer.m_buffer.data()), bufferSize);

		return is;
	}
}