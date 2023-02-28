#pragma once
#include <cstdint>
#include <vector>
#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <cassert>
#include <functional>
#include "assetHandle.h"
#include "assetFile.h"
#include "assetTexture.h"
#include "assetModel.h"
#include "assetMaterial.h"


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

		void AddNew(HandleIndex& index, const std::string& uri, uint16_t checkSum);
		bool UriExists(const std::string& uri) const;
		HandleIndex GetIndexFromUri(const std::string& uri) const;
		HandleChecksum GetChecksumFromIndex(HandleIndex index) const;
		HandleChecksum GetChecksumFromUri(const std::string& uri) const;

		virtual void Increase(HandleIndex size);
	private:
		std::queue<HandleIndex> m_free;

		std::unordered_map<std::string, HandleIndex> m_uriMap;
		std::deque<std::atomic<uint16_t>> m_refCount;
		std::vector<HandleChecksum> m_checkSums;
		friend struct AssetHandle;
	};

	template <typename T, typename UserT>
	class AssetManager : public BaseAssetManager
	{
	public:
		AssetManager();
		AssetHandle Load(const std::string& uri, bool keepFileData = true);

		bool Exists(const AssetHandle& handle);
		T* Get(const AssetHandle& handle);

		UserT* GetUserData(const AssetHandle& handle);

		void Release(HandleIndex index) override;

		void SetOnLoadCallback(std::function<void(const T&, UserT&)> onLoadCallback);
		void SetOnUnloadCallback(std::function<void(UserT&)> onUnloadCallback);
	protected:
		void Increase(HandleIndex size) override;
	private:
		std::vector<std::unique_ptr<T>> m_data;
		std::vector<UserT> m_userData;
		FileType fileType;

		std::function<void(const T&, UserT&)> m_onLoadCallback;
		std::function<void(UserT&)> m_onUnloadCallback;
	};

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::Increase(HandleIndex size)
	{
		BaseAssetManager::Increase(size);

		const HandleIndex previousSize = static_cast<HandleIndex>(m_data.size());
		const HandleIndex newSize = previousSize + size;

		m_data.resize(newSize);
		m_userData.resize(newSize);
	}

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::SetOnUnloadCallback(std::function<void(UserT&)> onUnloadCallback)
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
		else if (std::is_same<T, ModelInfo>::value)
		{
			fileType = FileType{ 'M','O','D','L' };
		}
		else if (std::is_same<T, MaterialInfo>::value)
		{
			fileType = FileType{ 'M','A','T','X' };
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

		return valid ? m_data.at(handle.Index()).get() : nullptr;
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
	AssetHandle Asset::AssetManager<T, UserT>::Load(const std::string& uri, bool keepFileData)
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



		HandleIndex index = static_cast<uint16_t>(m_data.size() - 1);
		AddNew(index, uri, file.checksum);

		m_data[index] = std::make_unique<T>(file);

		if (m_onLoadCallback)
			m_onLoadCallback(*m_data.at(index).get(), m_userData.at(index));

		if (!keepFileData)
			m_data[index].reset();

		return AssetHandle(index, file.checksum, this);
	}

	template <typename T, typename UserT>
	void Asset::AssetManager<T, UserT>::Release(HandleIndex index)
	{
		if (m_onUnloadCallback)
			m_onUnloadCallback(m_userData.at(index));

		BaseAssetManager::Release(index);
	}
}