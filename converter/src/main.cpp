#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

#include "jaam.h"

#include "nlohmann/json.hpp"
#include "lz4.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assetTexture.h"

#include "util.h"
#include "modelConverter.h"
using namespace Asset;


static uint16_t textureChecksum = 0;


bool ConvertImage(const fs::path& input, const fs::path& output, const fs::path& rootPath)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(input.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "Failed to load texture file " << input << std::endl;
		return false;
	}

	int texture_size = texWidth * texHeight * 4;

	TextureInfo texinfo;
	texinfo.textureSize = texture_size;
	texinfo.pixelsize[0] = texWidth;
	texinfo.pixelsize[1] = texHeight;
	texinfo.textureFormat = TextureFormat::RGBA8;
	texinfo.originalFile = GetRelativePathFrom(input, rootPath.string()).string();
	AssetFile newImage = PackTexture(&texinfo, pixels);
	newImage.checksum = textureChecksum++;

	stbi_image_free(pixels);

	newImage.SaveBinaryFile(output.string().c_str());

	return true;
}

struct TestUserData
{
	int a;
};

void OnLoad(const TextureInfo& info, TestUserData& userData)
{
	userData.a = 1;
}

void OnUnload(const TextureInfo& info, TestUserData& userData)
{
	userData.a = 0;
}

int main(int argc, char** argv)
{
	Asset::TextureManager<TestUserData> textureManager;
	textureManager.SetOnLoadCallback(&OnLoad);
	textureManager.SetOnUnloadCallback(&OnUnload);

	AssetHandle textureHandle = textureManager.Load("data/models/sponza/textures/background.tx");
	AssetHandle textureHandleLoad1 = textureManager.Load("data/models/sponza/textures/background.tx");

	Asset::TextureInfo* get = textureManager.Get(textureHandle);
	auto* userData = textureManager.GetUserData(textureHandle);
	std::cout << (int)get->textureFormat;

	for (int i = 0; i < argc; ++i)
		std::cout << argv[i] << '\n';

	const fs::path path{ argv[1] };
	const fs::path output{ argv[2] };

	std::cout << "loading asset directory at " << path << std::endl;

	for (auto& p : fs::recursive_directory_iterator(path))
	{
		const fs::path rootPath = path.filename();
		fs::path newpath = ChangeRoot(path, output, p.path());
		fs::path newdir = newpath;

		fs::create_directories(newdir.remove_filename());

		if (p.path().extension() == ".png") {
			std::cout << "found a texture" << p << std::endl;

			newpath.replace_extension(".tx");
			ConvertImage(p.path(), newpath, rootPath);
		}
		if (p.path().extension() == ".obj")
		{
			std::cout << "found a mesh" << p << std::endl;

			newpath.replace_extension(".mesh");
			ConvertMesh(p.path(), newpath, rootPath);
		}
	}
}