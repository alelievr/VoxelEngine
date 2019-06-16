#pragma once

#include <iostream>
#include <string>
#include "VoxIncludeDeps.hpp"

#include YAML_INCLUDE

// Settings classes
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
		~NoiseSettings(void) = default;
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

class ChunkLoaderSettings // TODO: parent class ?
{
	private:

	public:
		int				maxLoadPerFrame;
		int				maxLoadDistance; // distance is the radius of the loading sphere
		int				maxChunkCount;

		ChunkLoaderSettings(void) {}
		~ChunkLoaderSettings(void) = default;

		static void Load(const YAML::Node & node, ChunkLoaderSettings & settings)
		{
			settings.maxLoadPerFrame = node["maxLoadPerFrame"].as< int >();
			settings.maxLoadDistance = node["maxLoadDistance"].as< float >();
			settings.maxChunkCount = node["maxChunkCount"].as< float >();
		}

		// Serialization operators
		friend YAML::Emitter & operator<<(YAML::Emitter & out, const ChunkLoaderSettings & noiseSettings)
		{
			out << YAML::Key << "maxLoadPerFrame" << YAML::Value << noiseSettings.maxLoadPerFrame;
			out << YAML::Key << "maxLoadDistance" << YAML::Value << noiseSettings.maxLoadDistance;
			out << YAML::Key << "maxChunkCount" << YAML::Value << noiseSettings.maxChunkCount;

			return out;
		}
};

class		TerrainSettings
{
	private:

	public:
		std::vector< NoiseSettings >	noiseSettings;
		ChunkLoaderSettings				loaderSettings;
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
