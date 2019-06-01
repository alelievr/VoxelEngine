#include "AssetManager.hpp"

#include "IncludeDeps.hpp"
#include "Utils/Utils.hpp"
#include VULKAN_INCLUDE

#include "LWGC.hpp"
#include <iostream>
#include "dirent.h"

using namespace LWGC;

std::unordered_map< std::string, BlockData >	AssetManager::blockDatas;
Texture2DAtlas *								AssetManager::blockAtlas;

const std::string			AssetPath = "Assets";
const std::string			TexturesPath = AssetPath + "/Textures";
const std::string			BlockTexturesPath = TexturesPath + "/Blocks";
const std::string			EffectTexturesPath = TexturesPath + "/Effects";

const std::string			SideTexture = "_side";
const std::string			BottomTexture = "_bottom";
const std::string			TopTexture = "_top";

// All textures are in png
const std::string			ImageFormatExtension = "png";

void						AssetManager::LoadAssets()
{
	// Load all block datas
	std::vector< std::string > blockImagePathes;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(BlockTexturesPath.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (LWGC::GetExtension(ent->d_name) == ImageFormatExtension)
				blockImagePathes.push_back(std::string(BlockTexturesPath + "/" + ent->d_name));
		}
		closedir(dir);
	}
	else
	{
		throw std::runtime_error("Can't find the block texture asset folder");
	}

	// for (const auto & entry : fs::directory_iterator(BlockTexturesPath))
	std::sort(blockImagePathes.begin(), blockImagePathes.end());

	blockAtlas = Texture2DAtlas::Create(512, 512, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

	for (size_t id = 0; id < blockImagePathes.size(); id++)
	{
		std::string fileName = LWGC::GetFileNameWithoutExtension(blockImagePathes[id]);

		// We add 1 for ids because 0 is air
		blockDatas[fileName] = BlockData{
			static_cast< uint8_t >(id),
			TextureSide::All, // TODO
		};

		blockAtlas->Fit(blockImagePathes[id]);

		if (id >= MAX_BLOCK_ID - 1)
		{
			std::cerr << "Too many blocks in the Blocks texture folder !" << std::endl;
			break;
		}
	}

	blockAtlas->UploadAtlasDatas();
}

std::vector< BlockData >	AssetManager::GetBlockDatas(void)
{
	std::vector< BlockData >	blocks;

	for (const auto b : blockDatas)
	{
		blocks.push_back(b.second);
	}

	return blocks;
}
