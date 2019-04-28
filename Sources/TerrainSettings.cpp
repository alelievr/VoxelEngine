#include "TerrainSettings.hpp"

TerrainSettings::TerrainSettings(void)
{
	this->_chunkSize = 16;
	this->_seed = 0;
	// this->_noiseTree = ;
}

TerrainSettings::~TerrainSettings(void)
{
}

bool		TerrainSettings::Load(const std::string & fileName, TerrainSettings & settings) noexcept
{
	std::cout << "Load Terrain settings: TODO" << std::endl;
	return true;
}

bool		TerrainSettings::Save(const std::string & fileName) noexcept
{
	std::cout << "Save Terrain settings: TODO" << std::endl;
	return false;
}

size_t		TerrainSettings::GetChunkSize(void) const { return (this->_chunkSize); }
void		TerrainSettings::SetChunkSize(size_t tmp) { this->_chunkSize = tmp; }

int			TerrainSettings::GetSeed(void) const { return (this->_seed); }
void		TerrainSettings::SetSeed(int tmp) { this->_seed = tmp; }

std::vector< NoiseSettings >		TerrainSettings::GetNoiseSettings(void) const noexcept { return _noiseSettings; }
void		TerrainSettings::SetNoiseSettings(std::vector< NoiseSettings > tmp) noexcept { this->_noiseSettings = tmp; }

// NoiseTreeCSG		TerrainSettings::GetNoiseTree(void) const { return (this->_noiseTree); }
// void		TerrainSettings::SetNoiseTree(NoiseTreeCSG tmp) { this->_noiseTree = tmp; }

std::ostream &	operator<<(std::ostream & o, TerrainSettings const & r)
{
	o << "tostring of the class" << std::endl;
	(void)r;
	return (o);
}
