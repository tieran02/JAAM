#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <cassert>
#include "assetHandle.h"
#include "assetFile.h"


namespace Asset
{
	class BaseAssetManager
	{
	public:
		BaseAssetManager();
		virtual void Release(HandleIndex index);
	protected:
		void Reference(HandleIndex index);
		void Dereference(HandleIndex index);

		void AddNew(HandleIndex index, const std::string& uri, uint16_t checkSum);
		bool UriExists(const std::string& uri) const;
		HandleIndex GetIndexFromUri(const std::string& uri) const;
		HandleChecksum GetChecksumFromIndex(HandleIndex index) const;
		HandleChecksum GetChecksumFromUri(const std::string& uri) const;
	private:
		std::unordered_map<std::string, HandleIndex> m_uriMap;
		std::unordered_map<HandleIndex, std::atomic<uint16_t>> m_refCount;
		std::unordered_map<HandleIndex, HandleChecksum> m_checkSums;

		friend struct AssetHandle;
	};

	template <typename T, FileType ftype>
	class AssetManager : public BaseAssetManager
	{
	public:
		AssetHandle Load(const std::string& uri);

		bool Exists(const AssetHandle& handle);
		T* Get(const AssetHandle& handle);

		void Release(HandleIndex index) override;

	private:
		std::vector<std::unique_ptr<T>> m_data;
		const FileType fileType = ftype;
	};

	template <typename T, FileType ftype>
	T* Asset::AssetManager<T, ftype>::Get(const AssetHandle& handle)
	{
		if (handle == InvalidHandle)
			return nullptr;

		bool valid = handle.IsValid() && handle.Index() < m_data.size();
		assert(valid);

		//check if the checksum is the same, if not then the handle may be out of date 
		if (valid) 
		{
			valid |= handle.Checksum() == GetChecksumFromIndex(handle.Index());
			assert(valid);
		}

		return valid ? m_data.at(handle.Index()).get() : nullptr;
	}

	template <typename T, FileType ftype>
	AssetHandle Asset::AssetManager<T, ftype>::Load(const std::string& uri)
	{
		if (UriExists(uri))
		{
			return AssetHandle(GetIndexFromUri(uri), GetChecksumFromUri(uri), this);
		}

		//Load asset File
		AssetFile file;
		file.LoadBinaryFile(uri.c_str());

		//check if filetypes match
		bool fileTypeMatch = !memcmp(file.type.data(), fileType.data(), fileType.size());
		assert(fileTypeMatch);
		if (!fileTypeMatch)
			return InvalidHandle;

		auto data = std::make_unique<T>(file);


		m_data.emplace_back(std::move(data));
		HandleIndex index = static_cast<uint16_t>(m_data.size() - 1);
		AddNew(index, uri, file.checksum);

		return AssetHandle(index, file.checksum, this);;
	}

	template <typename T, FileType ftype>
	void Asset::AssetManager<T, ftype>::Release(HandleIndex index)
	{
		BaseAssetManager::Release(index);

		m_data.erase(m_data.begin() + index);
	}
}