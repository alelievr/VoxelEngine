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

	std::sort(blockImagePathes.begin(), blockImagePathes.end());

	blockAtlas = Texture2DAtlas::Create(512, 512, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

	// We start at 1 for ids because 0 is air
	size_t blockId = 0;

	for (size_t i = 0; i < blockImagePathes.size(); i++)
	{
		std::string fileName = LWGC::GetFileNameWithoutExtension(blockImagePathes[i]);
		TextureSide	side = TextureSide::All;

		blockAtlas->Fit(blockImagePathes[i]);

		// strip the name from side extensions:
		fileName = StripBlockImageExtension(fileName, side);

		// Add new block
		if (blockDatas.find(fileName) != blockDatas.end())
		{
			blockDatas[fileName] = BlockData { static_cast< uint8_t >(blockId), static_cast< uint8_t >(i), static_cast< uint8_t >(i), static_cast< uint8_t >(i) },
			blockId++;
		}

		// Patch block datas (with custom textures for side, bottom or top)
		switch (side)
		{
			case TextureSide::Side:
				blockDatas[fileName].sideAtlasIndex = i;
				break;
			case TextureSide::Bottom:
				blockDatas[fileName].bottomAtlasIndex = i;
				break;
			case TextureSide::Top:
				blockDatas[fileName].topAtlasIndex = i;
				break;
			default:
				break;
		}

		if (i >= MAX_BLOCK_ID - 1)
		{
			std::cerr << "Too many blocks in the Blocks texture folder !" << std::endl;
			break;
		}
	}

	// TODO: generate the mipmaps of the atlas

	blockAtlas->UploadAtlasDatas();
}

std::string					AssetManager::StripBlockImageExtension(std::string fileName, TextureSide & side)
{
	if (EndsWith(fileName, SideTexture))
	{
		fileName = fileName.substr(0, fileName.size() - SideTexture.size());
		side = TextureSide::Side;
	}
	else if (EndsWith(fileName, BottomTexture))
	{
		fileName = fileName.substr(0, fileName.size() - BottomTexture.size());
		side = TextureSide::Bottom;
	}
	else if (EndsWith(fileName, TopTexture))
	{
		fileName = fileName.substr(0, fileName.size() - TopTexture.size());
		side = TextureSide::Top;
	}
	return fileName;
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
