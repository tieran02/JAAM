#include "assetTexture.h"
#include "nlohmann/json.hpp"
#include "lz4.H"
#include "magic_enum.hpp"

using namespace Asset;


TextureInfo::TextureInfo() :
	textureSize(0),
	textureFormat(TextureFormat::Unknown),
	pixelsize{0,0,0}
{

}

TextureInfo::TextureInfo(const AssetFile& assetFile)
{
	*this = ReadTextureInfo(assetFile);
}

TextureInfo Asset::ReadTextureInfo(const AssetFile& file)
{
	TextureInfo info;

	nlohmann::json texture_metadata = nlohmann::json::parse(file.json);
	
	auto format = magic_enum::enum_cast<TextureFormat>(static_cast<std::string>(texture_metadata["format"]));
	info.textureFormat = format.value_or(TextureFormat::Unknown);

	info.pixelsize[0] = texture_metadata["width"];
	info.pixelsize[1] = texture_metadata["height"];
	info.textureSize = texture_metadata["buffer_size"];
	info.originalFile = texture_metadata["original_file"];

	info.data.resize(file.binaryBlob.TotalBufferSize());
	file.binaryBlob.CopyTo(info.data.data());

	return info;
}

AssetFile Asset::PackTexture(TextureInfo* info, void* pixelData)
{
	nlohmann::json texture_metadata;
	texture_metadata["format"] = magic_enum::enum_name(info->textureFormat);
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

	file.binaryBlob.CopyFrom(pixelData, info->textureSize, CompressionMode::LZ4);

	std::string stringified = texture_metadata.dump();
	file.json = stringified;

	return file;
}
