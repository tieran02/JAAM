#include "assetFile.h"
#include <fstream>
#include <filesystem>

using namespace Asset;


AssetFile::AssetFile() :
	type{0,0,0,0},
	version(0)
{

}

bool AssetFile::SaveBinaryFile(std::string_view path)
{
	std::filesystem::path jsonPath(path);
	jsonPath.replace_extension(jsonPath.extension().string() + ".meta");
	std::ofstream jsonFile;
	jsonFile.open(jsonPath, std::ios::out);
	jsonFile << json.data();
	jsonFile.close();

	std::ofstream binFile;
	binFile.open(path.data(), std::ios::binary | std::ios::out);

	binFile.write(type.data(), type.size());

	//version
	binFile.write(reinterpret_cast<const char*>(&version), sizeof(version));

	//checksum
	binFile.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

	//blob data
	binFile << binaryBlob;

	binFile.close();

	return true;
}

bool AssetFile::LoadBinaryFile(std::string_view path)
{
	std::filesystem::path jsonPath(path);
	jsonPath.replace_extension(jsonPath.extension().string() + ".meta");

	std::ifstream jsonFile;
	std::stringstream buffer;
	jsonFile.open(jsonPath, std::ios::in);
	if (!jsonFile.is_open()) return false;

	buffer << jsonFile.rdbuf();
	jsonFile.close();
	json = buffer.str();

	//Load binary file
	std::ifstream infile;
	infile.open(path.data(), std::ios::binary | std::ios::in);

	if (!infile.is_open()) return false;

	infile.read(type.data(), type.size());

	//version
	infile.read(reinterpret_cast<char*>(&version), sizeof(version));

	//checksum
	infile.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

	//blob data
	infile >> binaryBlob;

	return true;
}

CompressionMode Asset::ParseCompression(const char* f)
{
	if (strcmp(f, "LZ4") == 0)
	{
		return CompressionMode::LZ4;
	}
	else
	{
		return CompressionMode::None;
	}
}
