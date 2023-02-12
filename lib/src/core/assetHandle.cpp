#include "core/assetHandle.h"
#include "core/assetManager.h"

using namespace Asset;


AssetHandle::AssetHandle(HandleIndex index, HandleChecksum checksum, BaseAssetManager& assetManager) : m_manager(&assetManager)
{
	m_handle.m_index = index;
	m_handle.m_checksum = checksum;
	
	m_manager->Reference(index);
}

AssetHandle::AssetHandle(const AssetHandle& otherHandle)
{
	m_value = otherHandle.Value();
	m_manager = otherHandle.m_manager;

	m_manager->Reference(m_handle.m_index);
}

AssetHandle::~AssetHandle()
{
	m_manager->Dereference(m_handle.m_index);
}

HandleValue AssetHandle::Value() const
{
	return m_value;
}

HandleIndex AssetHandle::Index() const
{
	return m_handle.m_index;
}

HandleChecksum AssetHandle::Checksum() const
{
	return m_handle.m_checksum;
}

