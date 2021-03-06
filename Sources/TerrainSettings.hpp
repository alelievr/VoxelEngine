#pragma once

#include <iostream>
#include <string>
#include "VoxIncludeDeps.hpp"

#include YAML_INCLUDE

class NoiseSettings // TODO: parent class ?
{
	private:
		bool	_dirty;

		void	SetDirty(void) { _dirty = true; }

	public:
		int				octaves;
		float			turbulance;
		std::string		name;

		NoiseSettings(void) : _dirty(false) {}
		~NoiseSettings() = default;
		bool		IsDrity(void) const noexcept { return _dirty; }

		static void Load(const YAML::Node & node, NoiseSettings & settings)
		{
			settings.octaves = node["octaves"].as< int >();
			settings.turbulance = node["turbulance"].as< float >();
			settings.name = node["name"].as< std::string >();
		}

		// Serialization operators
		friend YAML::Emitter & operator<<(YAML::Emitter & out, const NoiseSettings & noiseSettings)
		{
			out << YAML::Key << "octaves" << YAML::Value << noiseSettings.octaves;
			out << YAML::Key << "turbulance" << YAML::Value << noiseSettings.turbulance;
			out << YAML::Key << "name" << YAML::Value << noiseSettings.name;

			return out;
		}
};

class		TerrainSettings
{
	private:

	public:
		std::vector< NoiseSettings >	noiseSettings;
		size_t							chunkSize;
		int								globalSeed;
		// NoiseTreeCSG	_noiseTree; // TODO

		TerrainSettings(void);
		TerrainSettings(const TerrainSettings &) = delete;
		virtual ~TerrainSettings(void);

		TerrainSettings &	operator=(TerrainSettings const & src) = delete;

		static bool	Load(const std::string & fileName, TerrainSettings & settings) noexcept;

		bool	Save(const std::string & fileName) noexcept;
};

std::ostream &	operator<<(std::ostream & o, TerrainSettings const & r);
