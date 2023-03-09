#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include "core/assetBuffer.h"

namespace Asset
{
	typedef std::array<char, 4> FileType;

	struct AssetFile
	{
		AssetFile();

		FileType type;
		uint32_t version;
		uint16_t checksum;
		std::string json;
		//std::vector<char> binaryBlob;
		Buffer binaryBlob;

		bool SaveBinaryFile(std::string_view path);
		bool LoadBinaryFile(std::string_view path);
	};

	CompressionMode ParseCompression(const char* f);
}