#pragma once
#include "assetFile.h"
#include <unordered_map>

namespace Asset
{
	enum class VertexDataType : uint16_t
	{
		PositionFloat2,
		PositionFloat3,
		NormalFloat3,
		ColorFloat2,
		ColorFloat3,
		TexCoordFloat2,
	};

	struct VertexBuffer
	{
		std::vector<VertexDataType> inputTypes;
		bool interleaved;

		std::vector<uint8_t> data;
		uint32_t GetStride() const;
	};

	typedef std::vector<uint32_t> IndexBuffer;
	typedef std::array<float, 16> Mat4x4;

	struct Mesh
	{
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
	};


	struct ModelInfo 
	{
		std::vector<std::string> meshNames;
		std::unordered_map<uint64_t, uint64_t> meshParents;  //Key = Mesh to get the parent for, Value = ParentId

		//Binary members
		std::vector<Mat4x4> transformMatrix;
		std::vector<Mesh> meshes;
	};


	ModelInfo ReadModelInfo(const AssetFile& file);
	AssetFile PackModel(const ModelInfo& info);
}