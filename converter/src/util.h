#pragma once
#include <filesystem>
namespace fs = std::filesystem;

fs::path ChangeRoot(const fs::path& orignalRoot, const fs::path& newRoot, const fs::path& path);
fs::path RemoveRootDir(const fs::path& orignalRoot, const fs::path& path);
fs::path GetRelativePathFrom(const fs::path& fullPath, const std::string& relativeFrom);
