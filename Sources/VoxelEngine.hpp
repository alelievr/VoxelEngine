#pragma once

#include <iostream>
#include <string>

#include "LWGC.hpp"
#include "TerrainSettings.hpp"

using namespace LWGC;

const std::string defaultConfigFile = "config/default.yml";

class		VoxelEngine
{
	private:
		GameObject *	_gui;

		void	RegisterEvents(EventSystem * es, Application * app);

	public:
		VoxelEngine(void) = delete;
		VoxelEngine(TerrainSettings & settingFile);

		VoxelEngine(const VoxelEngine &) = default;
		virtual ~VoxelEngine(void) = default;

		void	Load(TerrainSettings & settingFile);

		VoxelEngine &	operator=(VoxelEngine const & src) = delete;
};
