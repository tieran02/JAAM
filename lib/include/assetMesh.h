#pragma once
#include "assetFile.h"
#include <array>

namespace Asset
{
	/// <summary>
	/// Vertex Layout:
	///		Position: float32
	///		Normal: float32
	///		Colour: float32
	///		UV: float32
	/// </summary>
	struct Vertex_P32N32C32V32
	{
		std::array<float, 3> position;
		std::array<float, 3> normal;
		std::array<float, 3> colour;
		std::array<float, 2> uv;
	};

	enum class VertexFormat : uint32_t
	{
		Unknown = 0,
		P32N32C32V32, //everything at 32 bits
	};

	struct MeshInfo
	{
		MeshInfo();

		uint64_t vertexBuferSize;
		uint64_t indexBuferSize;
		VertexFormat vertexFormat;
		uint8_t vertexSize;
		uint8_t indexSize;
		CompressionMode compressionMode;
		std::string originalFile;
	};

	MeshInfo ReadMeshInfo(AssetFile* file);
	void UnpackMesh(MeshInfo* info, const char* sourcebuffer, size_t sourceSize, char* vertexBufer, char* indexBuffer);
	AssetFile PackMesh(MeshInfo* info, char* vertexData, char* indexData);
}