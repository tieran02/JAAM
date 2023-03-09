#include "assetMaterial.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;


MaterialInfo::MaterialInfo()
{

}

MaterialInfo::MaterialInfo(const AssetFile& assetFile)
{
	*this = ReadMaterialInfo(assetFile);
}

MaterialInfo Asset::ReadMaterialInfo(const AssetFile& file)
{
	MaterialInfo info;

	nlohmann::json material_metadata = nlohmann::json::parse(file.json);
	info.name = material_metadata["name"];
	info.baseEffect = material_metadata["baseEffect"];


	for (auto& [key, value] : material_metadata["textures"].items())
	{
		info.textures[key] = value;
	}

	info.floatParamters = material_metadata["floatParamters"];
	info.intParamters = material_metadata["intParamters"];
	info.vec3Paramters = material_metadata["float3Paramters"];
	info.vec4Paramters = material_metadata["float4Paramters"];

	info.transparency = TransparencyMode::Opaque;
	auto it = material_metadata.find("transparency");
	if (it != material_metadata.end())
	{
		std::string val = (*it);
		if (val.compare("opaque") == 0) {
			info.transparency = TransparencyMode::Opaque;
		}
		if (val.compare("transparent") == 0) {
			info.transparency = TransparencyMode::Transparent;
		}
		if (val.compare("masked") == 0) {
			info.transparency = TransparencyMode::Masked;
		}
	}

	return info;
}

AssetFile Asset::PackMaterial(MaterialInfo& info)
{
	nlohmann::json material_metadata;
	material_metadata["name"] = info.name;
	material_metadata["baseEffect"] = info.baseEffect;
	material_metadata["textures"] = info.textures;

	material_metadata["floatParamters"] = info.floatParamters;
	material_metadata["intParamters"] = info.intParamters;
	material_metadata["float3Paramters"] = info.vec3Paramters;
	material_metadata["float4Paramters"] = info.vec4Paramters;


	switch (info.transparency)
	{
	case TransparencyMode::Opaque:
		material_metadata["transparency"] = "opaque";
		break;
	case TransparencyMode::Transparent:
		material_metadata["transparency"] = "transparent";
		break;
	case TransparencyMode::Masked:
		material_metadata["transparency"] = "masked";
		break;
	}

	//core file header
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'A';
	file.type[2] = 'T';
	file.type[3] = 'X';
	file.version = 1;

	std::string stringified = material_metadata.dump();
	file.json = stringified;

	return file;
}
