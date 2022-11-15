#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace Asset
{
	struct AssetFile
	{
		AssetFile();

		std::array<char, 4> type;
		uint32_t version;
		std::string json;
		std::vector<char> binaryBlob;
	};

	enum class CompressionMode : uint32_t 
	{
		None,
		LZ4
	};

	bool SaveBinaryFile(const char* path, const AssetFile& file);
	bool LoadBinaryFile(const char* path, AssetFile& outputFile);

	CompressionMode ParseCompression(const char* f);
}