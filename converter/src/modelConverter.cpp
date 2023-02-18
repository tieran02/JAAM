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
			newFile.SaveBinaryFile(materialPath.string().c_str());
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
		std::function<void(aiNode* node, aiMatrix4x4& parentmat, uint64_t)> process_node = [&](aiNode* node, aiMatrix4x4& parentmat, uint64_t parentID) 
		{
			aiMatrix4x4 node_mat = /*parentmat * */node->mTransformation;

			uint64_t nodeindex = lastNode;
			lastNode++;

			if(parentID > 0)
				model.meshParents[nodeindex] = parentID;

			model.meshNames.emplace_back(node->mName.C_Str());
			model.transformMatrix.emplace_back(*reinterpret_cast<Mat4x4*>(&node->mTransformation));

			for (unsigned int msh = 0; msh < node->mNumMeshes; msh++)
			{
				Mesh mesh;
				mesh.vertexBuffer.inputTypes =
				{
					VertexDataType::PositionFloat3,
					VertexDataType::NormalFloat3,
					VertexDataType::TexCoordFloat2
				};
				mesh.vertexBuffer.interleaved = true;
				uint32_t stride = mesh.vertexBuffer.GetStride();

				auto aiMesh = scene->mMeshes[msh];

				mesh.vertexBuffer.data.resize(aiMesh->mNumVertices * stride);
				for (unsigned int v = 0; v < aiMesh->mNumVertices; v++)
				{
					size_t index = v * stride;
					memcpy(&mesh.vertexBuffer.data[index], &aiMesh->mVertices[v], sizeof(float) * 3);
					index += 3;

					memcpy(&mesh.vertexBuffer.data[index], &aiMesh->mNormals[v], sizeof(float) * 3);
					index += 3;

					if (aiMesh->GetNumUVChannels() >= 1)
					{
						memcpy(&mesh.vertexBuffer.data[index], &aiMesh->mTextureCoords[0][v], sizeof(float) * 2);
						index += 2;
					}
					else {
						mesh.vertexBuffer.data.at(index++) = 0;
						mesh.vertexBuffer.data.at(index++) = 0;
					}
				}

				mesh.indexBuffer.resize(aiMesh->mNumFaces * 3);
				for (unsigned int f = 0; f < aiMesh->mNumFaces; f++)
				{
					mesh.indexBuffer[f * 3 + 0] = aiMesh->mFaces[f].mIndices[0];
					mesh.indexBuffer[f * 3 + 1] = aiMesh->mFaces[f].mIndices[1];
					mesh.indexBuffer[f * 3 + 2] = aiMesh->mFaces[f].mIndices[2];
				}

				model.meshes.emplace_back(std::move(mesh));
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
		newFile.SaveBinaryFile(scenefilepath.string().c_str());
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
	const fs::path materialDir = outputDir.string() + "_materials";

	fs::create_directories(materialDir);

	bool success = ConvertAssimpMaterials(scene, input, materialDir, rootPath);
	success = ConvertNodes(scene, input, outputDir, rootPath);
	return success;
}
