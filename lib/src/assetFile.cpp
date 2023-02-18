#include "assetFile.h"
#include <fstream>

using namespace Asset;


AssetFile::AssetFile() :
	type{0,0,0,0},
	version(0)
{

}

bool AssetFile::SaveBinaryFile(const char* path)
{
	std::ofstream outfile;
	outfile.open(path, std::ios::binary | std::ios::out);

	outfile.write(type.data(), type.size());

	//version
	outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

	//checksum
	outfile.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

	//json
	size_t jsonSize = json.size();
	outfile.write(reinterpret_cast<const char*>(&jsonSize), sizeof(jsonSize));
	outfile.write(reinterpret_cast<const char*>(json.data()), jsonSize);

	//blob data
	outfile << binaryBlob;

	outfile.close();

	return true;
}

bool AssetFile::LoadBinaryFile(const char* path)
{
	std::ifstream infile;
	infile.open(path, std::ios::binary | std::ios::in);

	if (!infile.is_open()) return false;

	//move file cursor to beginning
	//infile.seekg(0);

	infile.read(type.data(), type.size());

	//version
	infile.read(reinterpret_cast<char*>(&version), sizeof(version));

	//checksum
	infile.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

	size_t jsonSize = 0;
	infile.read(reinterpret_cast<char*>(&jsonSize), sizeof(jsonSize));
	json.resize(jsonSize);
	infile.read(reinterpret_cast<char*>(json.data()), jsonSize);

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
