#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <cassert>
#include <functional>
#include "assetHandle.h"
#include "assetFile.h"
#include "assetTexture.h"


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

	template <typename T, typename UserT>
	class AssetManager : public BaseAssetManager
	{
	public:
		AssetManager();
		AssetHandle Load(const std::string& uri);

		bool Exists(const AssetHandle& handle);
		T* Get(const AssetHandle& handle);

		UserT* GetUserData(const AssetHandle& handle);

		void Release(HandleIndex index) override;

		void SetOnLoadCallback(std::function<void(const T&, UserT&)> onLoadCallback);
		void SetOnUnloadCallback(std::function<void(const T&, UserT&)> onUnloadCallback);

	private:
		std::vector<T> m_data;
		std::vector<UserT> m_userData;
		FileType fileType;

		std::function<void(const T&, UserT&)> m_onLoadCallback;
		std::function<void(const T&, UserT&)> m_onUnloadCallback;
	};

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::SetOnUnloadCallback(std::function<void(const T&, UserT&)> onUnloadCallback)
	{
		m_onUnloadCallback = onUnloadCallback;
	}

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::SetOnLoadCallback(std::function<void(const T&, UserT&)> onLoadCallback)
	{
		m_onLoadCallback = onLoadCallback;
	}

	template <typename T, typename UserT>
	Asset::AssetManager<T, UserT>::AssetManager()
	{
		if (std::is_same<T, TextureInfo>::value)
		{
			fileType = FileType{ 'T','E','X','I' };
		}
	}

	template <typename T, typename UserT>
	T* Asset::AssetManager<T, UserT>::Get(const AssetHandle& handle)
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

		return valid ? &m_data.at(handle.Index()) : nullptr;
	}

	template <typename T, typename UserT>
	UserT* Asset::AssetManager<T, UserT>::GetUserData(const AssetHandle& handle)
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

		return valid ? &m_userData.at(handle.Index()) : nullptr;
	}

	template <typename T, typename UserT>
	AssetHandle Asset::AssetManager<T, UserT>::Load(const std::string& uri)
	{
		if (UriExists(uri))
		{
			return AssetHandle(GetIndexFromUri(uri), GetChecksumFromUri(uri), this);
		}

		//Load asset File
		AssetFile file;
		if(!file.LoadBinaryFile(uri.c_str()))
			return InvalidHandle;


		//check if filetypes match
		bool fileTypeMatch = !memcmp(file.type.data(), fileType.data(), fileType.size());
		assert(fileTypeMatch);
		if (!fileTypeMatch)
			return InvalidHandle;


		m_data.emplace_back(file);
		HandleIndex index = static_cast<uint16_t>(m_data.size() - 1);
		AddNew(index, uri, file.checksum);

		m_userData.emplace_back();

		if (m_onLoadCallback)
			m_onLoadCallback(m_data.at(index), m_userData.at(index));

		return AssetHandle(index, file.checksum, this);
	}

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::Release(HandleIndex index)
	{
		if (m_onUnloadCallback)
			m_onUnloadCallback(m_data.at(index), m_userData.at(index));

		BaseAssetManager::Release(index);

		m_data.erase(m_data.begin() + index);
		m_userData.erase(m_userData.begin() + index);
	}
}