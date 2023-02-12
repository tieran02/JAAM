#pragma once
#include "assetFile.h"

namespace Asset
{
	enum class TextureFormat : uint32_t
	{
		Unknown = 0,
		RGBA8
	};

	struct TextureInfo 
	{
		TextureInfo();
		TextureInfo(const AssetFile& assetFile);

		int textureSize;
		TextureFormat textureFormat;
		CompressionMode compressionMode;
		std::array<uint32_t, 3> pixelsize; //[0] width [1] height [2] depth
		std::string originalFile;

		std::vector<char*> data;
	};

	TextureInfo ReadTextureInfo(const AssetFile& assetFile);
	void UnpackTexture(TextureInfo* info, const char* sourcebuffer, int sourceSize, char* destination);
	AssetFile PackTexture(TextureInfo* info, void* pixelData);
}