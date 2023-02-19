#pragma once
#include "core/assetHandle.h"
#include "core/assetManager.h"

#include "assetFile.h"
#include "assetTexture.h"
#include "assetMaterial.h"
#include "assetModel.h"

namespace Asset
{
	struct EmptyUserData {};
	using TextureManagerBasic = AssetManager<TextureInfo, EmptyUserData>;

	template <typename T>
	using TextureManager = AssetManager<TextureInfo, T>;


	using ModelManagerBasic = AssetManager<ModelInfo, EmptyUserData>;

	template <typename T>
	using ModelManager = AssetManager<ModelInfo, T>;
}