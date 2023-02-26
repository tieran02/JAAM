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

		std::string baseEffect;
		std::unordered_map<std::string, std::string >textures; // name/type -> path
		TransparencyMode transparency;
	};


	MaterialInfo ReadMaterialInfo(const AssetFile& file);
	AssetFile PackMaterial(MaterialInfo& info);
}