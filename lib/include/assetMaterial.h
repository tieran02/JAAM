#pragma once
#include "assetFile.h"
#include <unordered_map>

namespace Asset
{
	enum class TransparencyMode :uint8_t {
		Opaque,
		Transparent,
		Masked
	};


	struct MaterialInfo
	{
		MaterialInfo();
		MaterialInfo(const AssetFile& assetFile);

		std::string name;
		std::string baseEffect;
		std::unordered_map<std::string, std::string >textures; // name/type -> path

		std::unordered_map<std::string, float> floatParamters;
		std::unordered_map<std::string, int> intParamters;
		std::unordered_map<std::string, std::array<float, 3>> vec3Paramters;
		std::unordered_map<std::string, std::array<float, 4>> vec4Paramters;

		TransparencyMode transparency;
	};


	MaterialInfo ReadMaterialInfo(const AssetFile& file);
	AssetFile PackMaterial(MaterialInfo& info);
}