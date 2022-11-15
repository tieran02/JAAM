#include "util.h"
#include <filesystem>

fs::path ChangeRoot(const fs::path& orignalRoot, const fs::path& newRoot, const fs::path& path)
{
	return path.string().replace(0, orignalRoot.string().length(), newRoot.string());
}

fs::path RemoveRootDir(const fs::path& orignalRoot, const fs::path& path)
{
	return path.string().erase(0, orignalRoot.string().length());
}

fs::path GetRelativePathFrom(const fs::path& fullPath, const std::string& relativeFrom)
{
	std::string fullPathString = fullPath.string();
	auto it = fullPathString.find(relativeFrom);
	if (it != std::string::npos)
	{
		fullPathString.erase(0, it);
	}
	return fullPathString;
}
