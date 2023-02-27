#include "core/assetManager.h"
#include <cassert>

using namespace Asset;

BaseAssetManager::BaseAssetManager()
{

}

void BaseAssetManager::Reference(HandleIndex index)
{
	assert(index >= 0 && index < m_refCount.size()); // Index out of bounds
	m_refCount[index]++;
}

void BaseAssetManager::Dereference(HandleIndex index)
{
	assert(index >= 0 && index < m_refCount.size()); // Index out of bounds
	m_refCount[index]--;

	if (m_refCount[index] <= 0)
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

	//reset the ref count
	assert(index >= 0 && index < m_refCount.size()); // Index out of bounds
	m_refCount[index] = 0;

	//reset the checksum
	assert(index >= 0 && index < m_checkSums.size()); // Index out of bounds
	m_checkSums[index] = std::numeric_limits<HandleChecksum>::max();

	//add index back into the free queue
	m_free.push(index);
}

bool BaseAssetManager::UriExists(const std::string& uri) const
{
	return m_uriMap.find(uri) != m_uriMap.end();
}

void BaseAssetManager::AddNew(HandleIndex& index, const std::string& uri, uint16_t checkSum)
{
	//check if there are any free spaces, if not increase size
	if (m_free.empty())
		Increase(16);

	index = m_free.front();
	m_free.pop();

	m_uriMap.emplace(uri, index);

	assert(index >= 0 && index < m_refCount.size()); // Index out of bounds
	m_refCount[index] = 0;
	assert(index >= 0 && index < m_checkSums.size()); // Index out of bounds
	m_checkSums[index] = checkSum;
}

HandleIndex BaseAssetManager::GetIndexFromUri(const std::string& uri) const
{
	auto it = m_uriMap.find(uri);
	assert(it != m_uriMap.end());
	return it->second;
}

HandleChecksum BaseAssetManager::GetChecksumFromIndex(HandleIndex index) const
{
	assert(index >= 0 && index < m_checkSums.size()); // Index out of bounds
	return m_checkSums[index];
}

Asset::HandleChecksum BaseAssetManager::GetChecksumFromUri(const std::string& uri) const
{
	return GetChecksumFromIndex(GetIndexFromUri(uri));
}

void BaseAssetManager::Increase(HandleIndex size)
{
	const HandleIndex previousSize = static_cast<HandleIndex>(m_refCount.size());
	const HandleIndex newSize = previousSize + size;

	m_refCount.resize(newSize);
	m_checkSums.resize(newSize, std::numeric_limits<HandleChecksum>::max());

	for (HandleIndex i = previousSize; i < newSize; ++i)
	{
		m_free.push(i);
	}
}
