#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace Asset
{
	class BaseAssetManager;

	typedef uint32_t HandleValue;
	typedef uint16_t HandleIndex;
	typedef uint16_t HandleChecksum;


	/// <summary>
	/// Asset Handle is simply an interface to an asset, it stores a handle composed of an index to the underlaying array and a checksum to validate the asset type
	/// Handle is 32 bits: first 16 bits is the index, last 16 bits is the checksum
	/// </summary>
	struct AssetHandle
	{
		AssetHandle(HandleIndex index, HandleChecksum checksum, BaseAssetManager* assetManager);
		~AssetHandle();
		AssetHandle(const AssetHandle& otherHandle);

		HandleValue Value() const;
		HandleIndex Index() const;
		HandleChecksum Checksum() const;

		bool IsValid() const;

		bool operator==(const AssetHandle& rhs)
		{
			return Value() == rhs.Value();
		}

	private:
		union
		{
			HandleValue m_value;
			struct
			{
				HandleIndex m_index;
				HandleChecksum m_checksum;
			} m_handle;
		};

		BaseAssetManager* m_manager;
	};

	
	extern AssetHandle InvalidHandle;
}