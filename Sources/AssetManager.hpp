#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "VoxIncludeDeps.hpp"
#include "IncludeDeps.hpp"
#include GLM_INCLUDE
#include "LWGC.hpp"

#define MAX_BLOCK_ID	256

using namespace LWGC;

enum class	TextureSide
{
	All,
	Side,
	Top,
	Bottom,
};

struct		BlockData
{
	uint8_t		id;
	uint8_t		topAtlasIndex;
	uint8_t		sideAtlasIndex;
	uint8_t		bottomAtlasIndex;
};

class		AssetManager
{
	private:
		// TODO: integrate FMOD in the project
		// static std::unordered_map< std::string, SoundClass * > musics;
		static UniformBuffer	gpuBlockDatas;

		static std::string		StripBlockImageExtension(std::string fileName, TextureSide & side);
		static void				GenerateGPUBlockDatas(void);

	public:
		static std::unordered_map< std::string, BlockData >	blockDatas;
		static Texture2DAtlas *								blockAtlas;

		AssetManager(void) = default;
		AssetManager(const AssetManager &) = delete;
		virtual ~AssetManager(void) = default;

		static void LoadAssets();

		static VkBuffer	GetBlockDatasForGPU(void);

		AssetManager &	operator=(AssetManager const & src) = delete;
};
