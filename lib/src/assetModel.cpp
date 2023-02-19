#include "assetModel.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;

namespace
{
	size_t GetMeshDataSize(const std::vector<Mesh>& meshes)
	{
		return std::accumulate(meshes.begin(), meshes.end(), size_t(0), [](size_t sum, const Mesh& mesh) -> size_t
		{
			return sum +
			sizeof(uint8_t) + // input element count
			sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size() +
			sizeof(bool) + // interleaved
			sizeof(uint64_t) + // vertex buffer size;
			mesh.vertexBuffer.data.size();
		});
	}

	void PackMeshData(const std::vector<Mesh>& meshes, std::vector<char>& binaryBlob, size_t startIndex)
	{
		for (int i = 0; i < meshes.size(); ++i)
		{
			const Mesh& mesh = meshes[i];

			size_t srcIndex = startIndex;
			//pack input types
			binaryBlob[srcIndex] = static_cast<uint8_t>(mesh.vertexBuffer.inputTypes.size());
			srcIndex += sizeof(uint8_t);

			memcpy(&binaryBlob[srcIndex], mesh.vertexBuffer.inputTypes.data(), sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size());
			srcIndex += sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size();

			//interleaved
			binaryBlob[srcIndex] = mesh.vertexBuffer.interleaved;
			srcIndex += sizeof(bool);

			//vertex data
			uint64_t vertexCount = mesh.vertexBuffer.data.size();
			memcpy(&binaryBlob[srcIndex], &vertexCount, sizeof(uint64_t));
			srcIndex += sizeof(uint64_t);


			memcpy(&binaryBlob[srcIndex], mesh.vertexBuffer.data.data(), mesh.vertexBuffer.data.size());
			srcIndex += mesh.vertexBuffer.data.size();

		}
	}
}


ModelInfo Asset::ReadModelInfo(const AssetFile& file)
{
	ModelInfo info;
	nlohmann::json model_metadata = nlohmann::json::parse(file.json);

	info.meshNames = model_metadata["meshNames"];
	info.meshMaterials = model_metadata["meshMaterials"];
	info.meshParents = model_metadata["meshParents"];

	std::vector<char> tempBuffer(file.binaryBlob.TotalBufferSize());
	file.binaryBlob.CopyTo(tempBuffer.data());

	memcpy(info.transformMatrix.data(), tempBuffer.data(), info.transformMatrix.size() * sizeof(Mat4x4));

	return info;
}

AssetFile Asset::PackModel(const ModelInfo& info)
{
	nlohmann::json model_metadata;

	//core file header
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'O';
	file.type[2] = 'D';
	file.type[3] = 'L';
	file.version = 1;

	model_metadata["meshNames"] = info.meshNames;
	model_metadata["meshMaterials"] = info.meshMaterials;
	model_metadata["meshParents"] = info.meshParents;

	
	const size_t totalBlobSize = info.transformMatrix.size() * sizeof(Mat4x4) +
		GetMeshDataSize(info.meshes);

	std::vector<char> tempBuffer(totalBlobSize);

	memcpy(tempBuffer.data(), info.transformMatrix.data(), info.transformMatrix.size() * sizeof(Mat4x4));

	//now pack the mesh data
	PackMeshData(info.meshes, tempBuffer, info.transformMatrix.size() * sizeof(Mat4x4));

	file.binaryBlob.CopyFrom(tempBuffer.data(), tempBuffer.size(), CompressionMode::LZ4);

	std::string stringified = model_metadata.dump();
	file.json = stringified;

	return file;
}

uint32_t VertexBuffer::GetStride() const
{
	return std::accumulate(inputTypes.begin(), inputTypes.end(), 0u, [](uint32_t sum, VertexDataType type) -> uint32_t
	{
		switch (type)
		{
		case Asset::VertexDataType::PositionFloat2:
			return sum + (sizeof(float) * 2);
		case Asset::VertexDataType::PositionFloat3:
			return sum + (sizeof(float) * 3);
		case Asset::VertexDataType::NormalFloat3:
			return sum + (sizeof(float) * 3);
		case Asset::VertexDataType::ColorFloat2:
			return sum + (sizeof(float) * 2);
		case Asset::VertexDataType::ColorFloat3:
			return sum + (sizeof(float) * 3);
		case Asset::VertexDataType::TexCoordFloat2:
			return sum + (sizeof(float) * 2);
		}

		assert(false); // type doesn't have a size
		return sum;
	});
}
