#include "AssetManager.hpp"

#include "LWGC.hpp"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

using namespace LWGC;

std::unordered_map< std::string, BlockData > AssetManager::blockDatas;

const std::string			AssetPath = "Assets";
const std::string			TexturesPath = AssetPath + "/Textures";
const std::string			BlockTexturesPath = TexturesPath + "/Blocks";
const std::string			EffectTexturesPath = TexturesPath + "/Effects";

void						AssetManager::LoadAssets()
{
	// Load all block datas
	std::vector< std::string > blockImagePathes;
    for (const auto & entry : fs::directory_iterator(BlockTexturesPath))
		blockImagePathes.push_back(entry.path());
	std::sort(blockImagePathes.begin(), blockImagePathes.end());

	for (int id = 0; id < blockImagePathes.size(); id++)
	{
		string fileName = GetFileNameWithoutExtension(blockImagePathes[id]);

		// We add 1 for ids because 0 is air
		blockDatas[]

		if (id >= MAX_BLOCK_ID - 1)
		{
			std::cerr << "Too many blocks in the Blocks texture folder !"
			break;
		}
	}
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
