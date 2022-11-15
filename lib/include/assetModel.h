#pragma once
#include "assetFile.h"
#include <unordered_map>

namespace Asset
{
	struct ModelInfo 
	{
		//points to matrix array in the blob
		std::unordered_map<uint64_t, int> node_matrices;
		std::unordered_map<uint64_t, std::string> node_names;

		std::unordered_map<uint64_t, uint64_t> node_parents;

		struct NodeMesh {
			std::string material_path;
			std::string mesh_path;
		};

		std::unordered_map<uint64_t, NodeMesh> node_meshes;

		std::vector<std::array<float, 16>> matrices;
	};


	ModelInfo ReadModelInfo(AssetFile* file);
	AssetFile PackModel(const ModelInfo& info);
}