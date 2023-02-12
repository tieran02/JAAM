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
}

bool BaseAssetManager::UriExists(const std::string& uri) const
{
	return m_uriMap.find(uri) != m_uriMap.end();
}

void BaseAssetManager::AddNew(HandleIndex index, const std::string& uri)
{
	m_uriMap.emplace(uri, index);
	m_refCount.emplace(index, 0).first;
}

HandleIndex BaseAssetManager::GetIndexFromUri(const std::string& uri) const
{
	auto it = m_uriMap.find(uri);
	assert(it != m_uriMap.end());
	return it->second;
}
