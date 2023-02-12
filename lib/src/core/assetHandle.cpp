#include "core/assetHandle.h"
#include "core/assetManager.h"

using namespace Asset;
namespace Asset 
{
	AssetHandle InvalidHandle(std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max(), nullptr);
}

AssetHandle::AssetHandle(HandleIndex index, HandleChecksum checksum, BaseAssetManager* assetManager) : m_manager(assetManager)
{
	m_handle.m_index = index;
	m_handle.m_checksum = checksum;
	
	if(m_manager)
		m_manager->Reference(index);
}

AssetHandle::AssetHandle(const AssetHandle& otherHandle)
{
	m_value = otherHandle.Value();
	m_manager = otherHandle.m_manager;

	if (m_manager)
		m_manager->Reference(m_handle.m_index);
}

AssetHandle::~AssetHandle()
{
	if (m_manager)
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

bool AssetHandle::IsValid() const
{
	return *this != InvalidHandle;
}

