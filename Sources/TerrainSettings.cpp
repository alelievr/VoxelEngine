#include "TerrainSettings.hpp"

#include <fstream>

#include "VoxIncludeDeps.hpp"
#include YAML_INCLUDE

TerrainSettings::TerrainSettings(void)
{
	// Default parameters, will be used if there is a problem while loading the config file
	this->chunkSize = 128;
	this->globalSeed = 0;

	this->loaderSettings.maxChunkCount = 512;
	this->loaderSettings.maxLoadDistance = 16;
	this->loaderSettings.maxLoadPerFrame = 8;
}

TerrainSettings::~TerrainSettings(void)
{
}

bool		TerrainSettings::Load(const std::string & fileName, TerrainSettings & settings) noexcept
{
	try {
		// Note: think to update the Save function when you touch this one
		const auto & yaml = YAML::LoadFile(fileName);

		settings.globalSeed = yaml["globalSeed"].as<int>();
		settings.chunkSize = yaml["globalSeed"].as<int>();
		ChunkLoaderSettings::Load(yaml["loaderSettings"], settings.loaderSettings);

		for (const auto & node : yaml["noiseSettings"])
		{
			NoiseSettings ns;
			NoiseSettings::Load(node, ns);
			settings.noiseSettings.push_back(ns);
		}

		return true;
	}
	catch (const std::exception & e)
	{
		std::cerr << "Can't parse the config file: " << e.what() << std::endl;
		return false;
	}
}

bool		TerrainSettings::Save(const std::string & fileName) noexcept
{
	YAML::Emitter out;
	std::ofstream fout(fileName);

	if (!fout.is_open())
		return false;

	// Note: think to update the Load function when you touch this one
	out << YAML::Key << "chunkSize" << YAML::Value << chunkSize;
	out << YAML::Key << "globalSeed" << YAML::Value << globalSeed;
	out << YAML::Key << "loaderSettings" << YAML::Value << loaderSettings;

	// Noise Settings array:
	out << YAML::Key << "noiseSettings" << YAML::Value;
	out << YAML::BeginSeq;
	for (const auto & noiseSetting : noiseSettings)
	{
		out << noiseSetting;
	}
	out << YAML::EndSeq;

	fout << out.c_str();

	return true;
}

std::ostream &	operator<<(std::ostream & o, TerrainSettings const & r)
{
	o << "tostring of the class" << std::endl;
	(void)r;
	return (o);
}
