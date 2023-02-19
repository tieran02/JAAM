#include "assetModel.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;

namespace
{
	size_t GetMeshDataSize(const std::vector<Mesh>& meshes)
	{
		size_t size = sizeof(size_t); //Store the mesh count

		size += std::accumulate(meshes.begin(), meshes.end(), size_t(0), [](size_t sum, const Mesh& mesh) -> size_t
		{
			return sum +
			sizeof(uint8_t) + // input element count
			sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size() +
			sizeof(bool) + // interleaved
			sizeof(uint64_t) + // vertex buffer size;
			mesh.vertexBuffer.data.size() +
			sizeof(uint64_t) + // index buffer size;
			mesh.indexBuffer.size() * sizeof(uint32_t); //index is a 32bit uint
		});

		return size;
	}

	void PackMeshData(const std::vector<Mesh>& meshes, std::vector<char>& binaryBlob, size_t startIndex)
	{
		uint32_t meshCount = static_cast<uint32_t>(meshes.size());

		memcpy(&binaryBlob[startIndex], &meshCount, sizeof(meshCount));
		startIndex += sizeof(meshCount);

		for (int i = 0; i < meshes.size(); ++i)
		{
			const Mesh& mesh = meshes[i];

			size_t srcIndex = startIndex;

			//pack input types
			uint8_t inputTypeCount = static_cast<uint8_t>(mesh.vertexBuffer.inputTypes.size());
			memcpy(&binaryBlob[srcIndex], &inputTypeCount, sizeof(inputTypeCount));
			srcIndex += sizeof(uint8_t);

			memcpy(&binaryBlob[srcIndex], mesh.vertexBuffer.inputTypes.data(), sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size());
			srcIndex += sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size();

			//interleaved
			memcpy(&binaryBlob[srcIndex], &mesh.vertexBuffer.interleaved, sizeof(bool));
			srcIndex += sizeof(bool);

			//vertex data
			uint32_t vertexCount = static_cast<uint32_t>(mesh.vertexBuffer.data.size());
			memcpy(&binaryBlob[srcIndex], &vertexCount, sizeof(uint32_t));
			srcIndex += sizeof(uint32_t);


			memcpy(&binaryBlob[srcIndex], mesh.vertexBuffer.data.data(), mesh.vertexBuffer.data.size());
			srcIndex += mesh.vertexBuffer.data.size();

			//index data
			uint32_t indexCount = static_cast<uint32_t>(mesh.indexBuffer.size());
			memcpy(&binaryBlob[srcIndex], &indexCount, sizeof(uint32_t));
			srcIndex += sizeof(uint32_t);


			memcpy(&binaryBlob[srcIndex], mesh.indexBuffer.data(), mesh.indexBuffer.size() * sizeof(uint32_t));
			srcIndex += mesh.indexBuffer.size() * sizeof(uint32_t);

		}
	}

	void ReadMeshData(ModelInfo& info, std::vector<char>& binaryBlob, size_t startIndex)
	{
		uint32_t meshCount = 0;
		memcpy(&meshCount, &binaryBlob[startIndex], sizeof(meshCount));
		startIndex += sizeof(meshCount);

		info.meshes.resize(meshCount);
		for (uint32_t i = 0; i < meshCount; ++i)
		{
			Mesh& mesh = info.meshes[i];

			size_t srcIndex = startIndex;

			//pack input types
			uint8_t inputTypeCount = 0;
			memcpy(&inputTypeCount, &binaryBlob[srcIndex], sizeof(inputTypeCount));
			mesh.vertexBuffer.inputTypes.resize(inputTypeCount);
			srcIndex += sizeof(uint8_t);

			memcpy(mesh.vertexBuffer.inputTypes.data(), &binaryBlob[srcIndex], sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size());
			srcIndex += sizeof(VertexDataType) * mesh.vertexBuffer.inputTypes.size();

			//interleaved
			memcpy(&mesh.vertexBuffer.interleaved, &binaryBlob[srcIndex], sizeof(bool));
			srcIndex += sizeof(bool);

			//vertex data
			uint32_t vertexCount = 0;
			memcpy(&vertexCount, &binaryBlob[srcIndex], sizeof(uint32_t));
			mesh.vertexBuffer.data.resize(vertexCount);
			srcIndex += sizeof(uint32_t);


			memcpy(mesh.vertexBuffer.data.data(), &binaryBlob[srcIndex], mesh.vertexBuffer.data.size());
			srcIndex += mesh.vertexBuffer.data.size();

			//Read index data
			uint32_t indexCount = 0;
			memcpy(&indexCount, &binaryBlob[srcIndex], sizeof(uint32_t));
			mesh.indexBuffer.resize(indexCount);
			srcIndex += sizeof(uint32_t);


			memcpy(mesh.indexBuffer.data(), &binaryBlob[srcIndex], mesh.indexBuffer.size() * sizeof(uint32_t));
			srcIndex += mesh.indexBuffer.size() * sizeof(uint32_t);

		}
	}
}

ModelInfo::ModelInfo()
{

}

ModelInfo::ModelInfo(const AssetFile& assetFile)
{
	*this = ReadModelInfo(assetFile);
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

	info.transformMatrix.resize(info.meshNames.size());
	memcpy(info.transformMatrix.data(), tempBuffer.data(), info.transformMatrix.size() * sizeof(Mat4x4));

	ReadMeshData(info, tempBuffer, info.transformMatrix.size() * sizeof(Mat4x4));

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

size_t VertexBuffer::GetVertexCount() const
{
	return data.size() / GetStride();
}
