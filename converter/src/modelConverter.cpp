#include "modelConverter.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <functional>
#include <stdlib.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "jaam.h"
#include "util.h"

using namespace Asset;

namespace
{
	std::string AssimpMeshName(const aiScene* scene, int meshIndex)
	{
		std::string matname = "MESH_" + std::to_string(meshIndex) + "_" + std::string{ scene->mMeshes[meshIndex]->mName.C_Str() };
		return matname;
	}

	std::string AssimpMaterialName(const aiScene* scene, int materialIndex)
	{
		std::string matname = "MAT_" + std::to_string(materialIndex) + "_" + std::string{ scene->mMaterials[materialIndex]->GetName().C_Str() };
		return matname;
	}

	bool ConvertAssimpMaterials(const aiScene* scene, const fs::path& input, const fs::path& outputFolder, const fs::path& rootPath)
	{
		for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
			std::string matname = AssimpMaterialName(scene, m);

			MaterialInfo newMaterial;
			newMaterial.baseEffect = "default";

			aiMaterial* material = scene->mMaterials[m];
			newMaterial.transparency = TransparencyMode::Opaque;
			for (unsigned int p = 0; p < material->mNumProperties; p++)
			{
				aiMaterialProperty* pt = material->mProperties[p];
				switch (pt->mType)
				{
				case aiPTI_Float:
				{

					if (strcmp(pt->mKey.C_Str(), "$mat.opacity") == 0)
					{
						float num = *(float*)pt->mData;
						if (num != 1.0)
						{
							newMaterial.transparency = TransparencyMode::Transparent;
						}
					}
				}
				break;
				}
			}

			//check opacity
			std::string texPath = "";
			if (material->GetTextureCount(aiTextureType_DIFFUSE))
			{
				aiString assimppath;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &assimppath);

				fs::path texturePath = &assimppath.data[0];
				texPath = texturePath.string();
			}
			else if (material->GetTextureCount(aiTextureType_BASE_COLOR))
			{
				aiString assimppath;
				material->GetTexture(aiTextureType_BASE_COLOR, 0, &assimppath);

				fs::path texturePath = &assimppath.data[0];
				texPath = texturePath.string();
			}
			//force a default texture
			else 
			{
				texPath = "Default";
			}
			fs::path baseColorPath = outputFolder.parent_path() / texPath;

			baseColorPath.replace_extension(".tx");
			baseColorPath = GetRelativePathFrom(baseColorPath, rootPath.string());

			newMaterial.textures["baseColor"] = baseColorPath.string();

			fs::path materialPath = outputFolder / (matname + ".mat");

			AssetFile newFile = PackMaterial(&newMaterial);

			//save to disk
			SaveBinaryFile(materialPath.string().c_str(), newFile);
		}
		return true;
	}

	bool ConvertAssimpMesh(const aiScene* scene, const fs::path& input, const fs::path& outputFolder, const fs::path& rootPath)
	{
		for (unsigned int meshindex = 0; meshindex < scene->mNumMeshes; meshindex++) {

			auto mesh = scene->mMeshes[meshindex];

			auto VertexFormatEnum = VertexFormat::P32N32C32V32;
			using VertexFormatType = Vertex_P32N32C32V32;

			std::vector<VertexFormatType> _vertices;
			std::vector<uint32_t> _indices;

			std::string meshname = AssimpMeshName(scene, meshindex);

			_vertices.resize(mesh->mNumVertices);
			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				VertexFormatType vert;
				vert.position[0] = mesh->mVertices[v].x;
				vert.position[1] = mesh->mVertices[v].y;
				vert.position[2] = mesh->mVertices[v].z;

				vert.normal[0] = mesh->mNormals[v].x;
				vert.normal[1] = mesh->mNormals[v].y;
				vert.normal[2] = mesh->mNormals[v].z;

				if (mesh->GetNumUVChannels() >= 1)
				{
					vert.uv[0] = mesh->mTextureCoords[0][v].x;
					vert.uv[1] = mesh->mTextureCoords[0][v].y;
				}
				else {
					vert.uv[0] = 0;
					vert.uv[1] = 0;
				}
				if (mesh->HasVertexColors(0))
				{
					vert.colour[0] = mesh->mColors[0][v].r;
					vert.colour[1] = mesh->mColors[0][v].g;
					vert.colour[2] = mesh->mColors[0][v].b;
				}
				else {
					vert.colour[0] = 1;
					vert.colour[1] = 1;
					vert.colour[2] = 1;
				}

				_vertices[v] = vert;
			}
			_indices.resize(mesh->mNumFaces * 3);
			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				_indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
				_indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
				_indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
			}

			MeshInfo meshinfo;
			meshinfo.vertexFormat = VertexFormatEnum;
			meshinfo.vertexBuferSize = _vertices.size() * sizeof(VertexFormatType);
			meshinfo.indexBuferSize = _indices.size() * sizeof(uint32_t);
			meshinfo.vertexSize = sizeof(VertexFormatType);
			meshinfo.indexSize = sizeof(uint32_t);
			meshinfo.originalFile = GetRelativePathFrom(input, rootPath.string()).string();

			AssetFile newFile = PackMesh(&meshinfo, (char*)_vertices.data(), (char*)_indices.data());

			fs::path meshpath = outputFolder / (meshname + ".mesh");

			//save to disk
			SaveBinaryFile(meshpath.string().c_str(), newFile);
		}
		return true;
	}

	bool ConvertNodes(const aiScene* scene, const fs::path& input, const fs::path& outputFolder, const fs::path& rootPath)
	{
		ModelInfo model;

		glm::mat4 ident{ 1.f };

		std::array<float, 16> identityMatrix;
		memcpy(&identityMatrix, &ident, sizeof(glm::mat4));


		uint64_t lastNode = 0;
		std::function<void(aiNode* node, aiMatrix4x4& parentmat, uint64_t)> process_node = [&](aiNode* node, aiMatrix4x4& parentmat, uint64_t parentID) {

			aiMatrix4x4 node_mat = /*parentmat * */node->mTransformation;

			glm::mat4 modelmat;
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					modelmat[y][x] = node_mat[x][y];
				}
			}

			uint64_t nodeindex = lastNode;
			lastNode++;

			std::array<float, 16> matrix;
			memcpy(&matrix, &modelmat, sizeof(glm::mat4));

			if (parentID != nodeindex)
			{
				model.node_parents[nodeindex] = parentID;
			}

			model.node_matrices[nodeindex] = model.matrices.size();
			model.matrices.push_back(matrix);


			std::string nodename = node->mName.C_Str();
			//std::cout << nodename << std::endl;

			if (nodename.size() > 0)
			{
				model.node_names[nodeindex] = nodename;
			}
			for (unsigned int msh = 0; msh < node->mNumMeshes; msh++) {

				int mesh_index = node->mMeshes[msh];
				std::string meshname = "Mesh: " + std::string{ scene->mMeshes[mesh_index]->mName.C_Str() };

				//std::cout << meshname << std::endl;

				std::string matname = AssimpMaterialName(scene, scene->mMeshes[mesh_index]->mMaterialIndex);
				meshname = AssimpMeshName(scene, mesh_index);

				fs::path materialpath = (outputFolder.string() + "_materials/" + matname + ".mat");
				fs::path meshpath = (outputFolder.string() + "_meshes/" + meshname + ".mesh");

				materialpath = GetRelativePathFrom(materialpath, rootPath.string());
				meshpath = GetRelativePathFrom(meshpath, rootPath.string());


				ModelInfo::NodeMesh nmesh;
				nmesh.mesh_path = meshpath.string();
				nmesh.material_path = materialpath.string();
				uint64_t newNode = lastNode; lastNode++;

				model.node_meshes[newNode] = nmesh;
				model.node_parents[newNode] = nodeindex;

				model.node_matrices[newNode] = model.matrices.size();
				model.matrices.push_back(identityMatrix);
			}

			for (unsigned int ch = 0; ch < node->mNumChildren; ch++)
			{
				process_node(node->mChildren[ch], node_mat, nodeindex);
			}
		};

		aiMatrix4x4 mat{};
		glm::mat4 rootmat{ 1 };// (, rootMatrix.v);

		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				mat[x][y] = rootmat[y][x];
			}
		}

		process_node(scene->mRootNode, mat, 0);

		AssetFile newFile = PackModel(model);

		fs::path scenefilepath = (outputFolder.parent_path()) / input.stem();

		scenefilepath.replace_extension(".modl");

		//save to disk
		SaveBinaryFile(scenefilepath.string().c_str(), newFile);
		return true;
	}

}


bool ConvertMesh(const fs::path& input, const fs::path& outputFolder, const fs::path& rootPath)
{
	// Check if file exists
	std::ifstream fin(input.string().c_str());
	if (fin.fail())
	{
		return false;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(input.string().c_str(), aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs);

	fs::path outputDir = outputFolder;
	outputDir.replace_extension();
	const fs::path meshDir = outputDir.string() + "_meshes";
	const fs::path materialDir = outputDir.string() + "_materials";

	fs::create_directories(meshDir);
	fs::create_directories(materialDir);

	bool success = ConvertAssimpMesh(scene, input, meshDir, rootPath);
	success = ConvertAssimpMaterials(scene, input, materialDir, rootPath);
	success = ConvertNodes(scene, input, outputDir, rootPath);
	return success;
}
