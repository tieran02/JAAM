#include "core/assetHandle.h"
#include "core/assetManager.h"

#include "assetFile.h"
#include "assetTexture.h"
#include "assetMaterial.h"
#include "assetModel.h"

namespace Asset
{
	struct EmptyUserData {};

	template <typename T>
	using TextureManager = AssetManager<TextureInfo, T>;
}