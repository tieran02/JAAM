#include "assetTexture.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;

namespace
{
	TextureFormat ParseFormat(const char* f)
	{
		if (strcmp(f, "RGBA8") == 0)
		{
			return TextureFormat::RGBA8;
		}
		else 
		{
			return TextureFormat::Unknown;
		}
	}
}

TextureInfo::TextureInfo() :
	textureSize(0),
	textureFormat(TextureFormat::Unknown),
	compressionMode(CompressionMode::LZ4),
	pixelsize{0,0,0}
{

}

TextureInfo Asset::ReadTextureInfo(AssetFile* file)
{
	TextureInfo info;

	nlohmann::json texture_metadata = nlohmann::json::parse(file->json);

	std::string formatString = texture_metadata["format"];
	info.textureFormat = ParseFormat(formatString.c_str());

	std::string compressionString = texture_metadata["compression"];
	info.compressionMode = ParseCompression(compressionString.c_str());

	info.pixelsize[0] = texture_metadata["width"];
	info.pixelsize[1] = texture_metadata["height"];
	info.textureSize = texture_metadata["buffer_size"];
	info.originalFile = texture_metadata["original_file"];

	return info;
}

void Asset::UnpackTexture(TextureInfo* info, const char* sourcebuffer, int sourceSize, char* destination)
{
	if (info->compressionMode == CompressionMode::LZ4) 
	{
		LZ4_decompress_safe(sourcebuffer, destination, sourceSize, info->textureSize);
	}
	else
	{
		memcpy(destination, sourcebuffer, sourceSize);
	}
}

AssetFile Asset::PackTexture(TextureInfo* info, void* pixelData)
{
	nlohmann::json texture_metadata;
	texture_metadata["format"] = "RGBA8";
	texture_metadata["width"] = info->pixelsize[0];
	texture_metadata["height"] = info->pixelsize[1];
	texture_metadata["buffer_size"] = info->textureSize;
	texture_metadata["original_file"] = info->originalFile;

	//core file header
	AssetFile file;
	file.type[0] = 'T';
	file.type[1] = 'E';
	file.type[2] = 'X';
	file.type[3] = 'I';
	file.version = 1;

	//compress buffer into blob
	int compressStaging = LZ4_compressBound(info->textureSize);

	file.binaryBlob.resize(compressStaging);

	int compressedSize = LZ4_compress_default((const char*)pixelData, file.binaryBlob.data(), info->textureSize, compressStaging);

	file.binaryBlob.resize(compressedSize);

	texture_metadata["compression"] = "LZ4";

	std::string stringified = texture_metadata.dump();
	file.json = stringified;

	return file;
}
