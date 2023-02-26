#include "core/assetManager.h"
#include <cassert>

using namespace Asset;

BaseAssetManager::BaseAssetManager()
{

}

void BaseAssetManager::Reference(HandleIndex index)
{
	auto it = m_refCount.find(index);
	assert(it != m_refCount.end());

	it->second++;
}

void BaseAssetManager::Dereference(HandleIndex index)
{
	auto it = m_refCount.find(index);
	assert(it != m_refCount.end());

	it->second--;

	if (it->second <= 0)
	{
		Release(index);
	}
}

void BaseAssetManager::Release(HandleIndex index)
{
	//remove the cached uri from the map, need to loop through all elements so not the fastest
	auto it = std::find_if(m_uriMap.begin(), m_uriMap.end(), [&index](const auto& p) { return p.second == index; });
	assert(it != m_uriMap.end());
	m_uriMap.erase(it);

	//Remove the ref count from refCount map
	m_refCount.erase(index);

	//Remove the checksum
	m_checkSums.erase(index);
}

bool BaseAssetManager::UriExists(const std::string& uri) const
{
	return m_uriMap.find(uri) != m_uriMap.end();
}

void BaseAssetManager::AddNew(HandleIndex index, const std::string& uri, uint16_t checkSum)
{
	m_uriMap.emplace(uri, index);
	m_refCount.emplace(index, static_cast<uint16_t>(0));
	m_checkSums.emplace(index, checkSum);
}

HandleIndex BaseAssetManager::GetIndexFromUri(const std::string& uri) const
{
	auto it = m_uriMap.find(uri);
	assert(it != m_uriMap.end());
	return it->second;
}

HandleChecksum BaseAssetManager::GetChecksumFromIndex(HandleIndex index) const
{
	auto it = m_checkSums.find(index);
	assert(it != m_checkSums.end());
	return it->second;
}

Asset::HandleChecksum BaseAssetManager::GetChecksumFromUri(const std::string& uri) const
{
	return GetChecksumFromIndex(GetIndexFromUri(uri));
}