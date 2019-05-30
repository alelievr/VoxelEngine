#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "VoxIncludeDeps.hpp"
#include "IncludeDeps.hpp"
#include GLM_INCLUDE

#define MAX_BLOCK_ID	256

enum class	TextureSide
{
	All,
	Side,
	Top,
	Bottom,
}

struct		BlockData
{
	uint8_t		id;
	TextureSide	side;
	glm::vec4	atlasDatas;
};

class		AssetManager
{
	private:
		// TODO: integrate FMOD in the project
		// static std::unordered_map< std::string, SoundClass * > musics;

	public:
		static std::unordered_map< std::string, BlockData >	blockDatas;

		AssetManager(void) = default;
		AssetManager(const AssetManager &) = delete;
		virtual ~AssetManager(void) = default;

		static void LoadAssets();

		static std::vector< BlockData >	GetBlockDatas(void);

		AssetManager &	operator=(AssetManager const & src) = delete;
};
