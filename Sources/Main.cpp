#include <vector>
#include <iostream>

#include "TerrainSettings.hpp"
#include "VoxelEngine.hpp"

using namespace LWGC;

const std::string defaultConfigFile = "config/default.yml";

int		main(int ac, char **av)
{
	std::string configFile;

	if (ac == 1)
		configFile = defaultConfigFile;
	else
		configFile = std::string(av[1]);

	TerrainSettings settings = TerrainSettings::Load(configFile);

	VoxelEngine engine(settings);
}
