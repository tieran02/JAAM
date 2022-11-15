#include "assetMesh.h"
#include "nlohmann/json.hpp"
#include "lz4.H"

using namespace Asset;

namespace
{
	VertexFormat ParseFormat(const char* f) 
	{

		if (strcmp(f, "P32N32C32V32") == 0)
		{
			return VertexFormat::P32N32C32V32;
		}
		else
		{
			return VertexFormat::Unknown;
		}
	}
}


MeshInfo::MeshInfo() :
	vertexBuferSize(0),
	indexBuferSize(0),
	vertexFormat(VertexFormat::P32N32C32V32),
	vertexSize(0),
	indexSize(0),
	compressionMode(CompressionMode::LZ4)
{

}

MeshInfo Asset::ReadMeshInfo(AssetFile* file)
{
	MeshInfo info;

	nlohmann::json metadata = nlohmann::json::parse(file->json);

	info.vertexBuferSize = metadata["vertex_buffer_size"];
	info.indexBuferSize = metadata["index_buffer_size"];
	info.vertexSize = (uint8_t)metadata["vertex_size"];
	info.indexSize = (uint8_t)metadata["index_size"];
	info.originalFile = metadata["original_file"];

	std::string compressionString = metadata["compression"];
	info.compressionMode = ParseCompression(compressionString.c_str());

	std::string vertexFormat = metadata["vertex_format"];
	info.vertexFormat = ParseFormat(vertexFormat.c_str());

	return info;
}

void Asset::UnpackMesh(MeshInfo* info, const char* sourcebuffer, size_t sourceSize, char* vertexBufer, char* indexBuffer)
{
	std::vector<char> decompressedBuffer;
	decompressedBuffer.resize(info->vertexBuferSize + info->indexBuferSize);

	LZ4_decompress_safe(sourcebuffer, decompressedBuffer.data(), static_cast<int>(sourceSize), static_cast<int>(decompressedBuffer.size()));

	//copy vertex buffer
	memcpy(vertexBufer, decompressedBuffer.data(), info->vertexBuferSize);

	////copy index buffer
	memcpy(indexBuffer, &decompressedBuffer[info->vertexBuferSize], info->indexBuferSize);

	decompressedBuffer.clear();
}

AssetFile Asset::PackMesh(MeshInfo* info, char* vertexData, char* indexData)
{
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'E';
	file.type[2] = 'S';
	file.type[3] = 'H';
	file.version = 1;

	nlohmann::json metadata;
	if (info->vertexFormat == VertexFormat::P32N32C32V32) {
		metadata["vertex_format"] = "P32N32C32V32";
	}

	metadata["vertex_buffer_size"] = info->vertexBuferSize;
	metadata["index_buffer_size"] = info->indexBuferSize;
	metadata["vertex_size"] = info->vertexSize;
	metadata["index_size"] = info->indexSize;
	metadata["original_file"] = info->originalFile;

	size_t fullsize = info->vertexBuferSize + info->indexBuferSize;

	std::vector<char> merged_buffer;
	merged_buffer.resize(fullsize);

	//copy vertex buffer
	memcpy(merged_buffer.data(), vertexData, info->vertexBuferSize);

	//copy index buffer
	memcpy(merged_buffer.data() + info->vertexBuferSize, indexData, info->indexBuferSize);


	//compress buffer and copy it into the file struct
	size_t compressStaging = LZ4_compressBound(static_cast<int>(fullsize));

	file.binaryBlob.resize(compressStaging);

	int compressedSize = LZ4_compress_default(merged_buffer.data(), file.binaryBlob.data(), static_cast<int>(merged_buffer.size()), static_cast<int>(compressStaging));
	file.binaryBlob.resize(compressedSize);

	metadata["compression"] = "LZ4";

	file.json = metadata.dump();

	return file;
}
