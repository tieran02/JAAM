#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>
#include "assetHandle.h"


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

		void AddNew(HandleIndex index, const std::string& uri);
		bool UriExists(const std::string& uri) const;
		HandleIndex GetIndexFromUri(const std::string& uri) const;
	private:
		std::unordered_map<std::string, HandleIndex> m_uriMap;
		std::unordered_map<HandleIndex, std::atomic<uint16_t>> m_refCount;

		friend struct AssetHandle;
	};

	template <typename T>
	class AssetManager : public BaseAssetManager
	{
	public:
		AssetHandle Load(const std::string& uri);
		void Release(HandleIndex index) override;

	private:
		std::vector<std::unique_ptr<T>> m_data;
	};

	template <typename T>
	AssetHandle Asset::AssetManager<T>::Load(const std::string& uri)
	{
		uint16_t checksum = 0;

		if (UriExists(uri))
		{
			return AssetHandle(GetIndexFromUri(uri), checksum, *this);
		}

		//Load from file
		std::unique_ptr<T> temp = std::make_unique<T>();

		m_data.emplace_back(std::move(temp));
		HandleIndex index = static_cast<uint16_t>(m_data.size() - 1);
		AddNew(index, uri);

		return AssetHandle(index, checksum, *this);;
	}

	template <typename T>
	void Asset::AssetManager<T>::Release(HandleIndex index)
	{
		BaseAssetManager::Release(index);

		m_data.erase(m_data.begin() + index);
	}

}