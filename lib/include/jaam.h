#include "core/assetHandle.h"
#include "core/assetManager.h"

#include "assetFile.h"
#include "assetTexture.h"
#include "assetMesh.h"
#include "assetMaterial.h"
#include "assetModel.h"

namespace Asset
{
	typedef AssetManager < TextureInfo, FileType{ 'T','E','X','I' } > TextureManager;
}