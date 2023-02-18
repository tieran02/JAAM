#pragma once
#include "assetFile.h"
#include "nlohmann/json.hpp"

namespace Asset
{
	enum class TextureFormat
	{
		Unknown,
		RGBA8
	};

	struct TextureInfo 
	{
		TextureInfo();
		TextureInfo(const AssetFile& assetFile);

		int textureSize;
		TextureFormat textureFormat;
		std::array<uint32_t, 3> pixelsize; //[0] width [1] height [2] depth
		std::string originalFile;

		std::vector<uint8_t> data;
	};

	TextureInfo ReadTextureInfo(const AssetFile& assetFile);
	AssetFile PackTexture(TextureInfo* info, void* pixelData);
}