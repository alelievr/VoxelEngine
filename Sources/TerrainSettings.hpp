#pragma once

#include <iostream>
#include <string>
#include "IncludeDeps.hpp"

#include YAML_INCLUDE

class NoiseSettings // TODO: parent class ?
{
	private:
		int		_octaves;
		float	_turbulance;
		bool	_dirty;

		void	SetDirty(void) { _dirty = true; }

	public:
		NoiseSettings(void) : _dirty(false) {}
		~NoiseSettings() = default;

		int		GetOctaves(void) const noexcept { return _octaves; }
		float	GetTurbulance(void) const noexcept { return _turbulance; }
		void	SetOctaves(int octaves) noexcept { _octaves = octaves; SetDirty(); }
		void	SetTurbulance(float turbulance) noexcept { _turbulance = turbulance; SetDirty(); }
		bool	IsDrity(void) const noexcept { return _dirty; }
};

// TODO: << and >> operator for yaml lib

class		TerrainSettings
{
	private:
		size_t	_chunkSize;
		int	_seed;
		std::vector< NoiseSettings >	_noiseSettings;
		// NoiseTreeCSG	_noiseTree; // TODO


	public:
		TerrainSettings(void);
		TerrainSettings(const TerrainSettings &) = delete;
		virtual ~TerrainSettings(void);

		TerrainSettings &	operator=(TerrainSettings const & src) = delete;

		static TerrainSettings	Load(const std::string & fileName) noexcept;

		bool	Save(const std::string & fileName) noexcept;

		size_t	GetChunkSize(void) const;
		void	SetChunkSize(size_t tmp);

		int		GetSeed(void) const;
		void	SetSeed(int tmp);

		std::vector< NoiseSettings >	GetNoiseSettings(void) const noexcept;
		void	SetNoiseSettings(std::vector< NoiseSettings > tmp) noexcept;

		// NoiseTreeCSG	GetNoiseTree(void) const;
		// void	SetNoiseTree(NoiseTreeCSG tmp);
};

std::ostream &	operator<<(std::ostream & o, TerrainSettings const & r);
