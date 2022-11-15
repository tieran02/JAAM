#include "assetModel.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;

ModelInfo Asset::ReadModelInfo(AssetFile* file)
{
	ModelInfo info;
	nlohmann::json model_metadata = nlohmann::json::parse(file->json);

	for (auto pair : model_metadata["node_matrices"].items())
	{
		auto value = pair.value();
		auto k = pair.key();
		info.node_matrices[value[0]] = value[1];
	}

	for (auto& [key, value] : model_metadata["node_names"].items())
	{
		info.node_names[value[0]] = value[1];
	}

	for (auto& [key, value] : model_metadata["node_parents"].items())
	{

		info.node_parents[value[0]] = value[1];
	}

	std::unordered_map<uint64_t, nlohmann::json> meshnodes = model_metadata["node_meshes"];

	for (auto pair : meshnodes)
	{
		ModelInfo::NodeMesh node;

		node.mesh_path = pair.second["mesh_path"];
		node.material_path = pair.second["material_path"];

		info.node_meshes[pair.first] = node;
	}

	size_t nmatrices = file->binaryBlob.size() / (sizeof(float) * 16);
	info.matrices.resize(nmatrices);

	memcpy(info.matrices.data(), file->binaryBlob.data(), file->binaryBlob.size());

	return info;
}

AssetFile Asset::PackModel(const ModelInfo& info)
{
	nlohmann::json model_metadata;
	model_metadata["node_matrices"] = info.node_matrices;
	model_metadata["node_names"] = info.node_names;
	model_metadata["node_parents"] = info.node_parents;

	std::unordered_map<uint64_t, nlohmann::json> meshindex;
	for (auto pair : info.node_meshes)
	{
		nlohmann::json meshnode;
		meshnode["mesh_path"] = pair.second.mesh_path;
		meshnode["material_path"] = pair.second.material_path;
		meshindex[pair.first] = meshnode;
	}

	model_metadata["node_meshes"] = meshindex;

	//core file header
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'O';
	file.type[2] = 'D';
	file.type[3] = 'L';
	file.version = 1;

	file.binaryBlob.resize(info.matrices.size() * sizeof(float) * 16);
	memcpy(file.binaryBlob.data(), info.matrices.data(), info.matrices.size() * sizeof(float) * 16);

	std::string stringified = model_metadata.dump();
	file.json = stringified;

	return file;
}
