#pragma once

#include <string>

#include "LWGC.hpp"
#include "TerrainSettings.hpp"
#include "ChunkRenderer.hpp"
#include "ChunkMap.hpp"

#include "IncludeDeps.hpp"
#include GLM_INCLUDE

using namespace LWGC;

class ChunkLoader
{
	private:
		SwapChain *			_swapChain;
		ChunkMap			_loadedChunks;

		ComputeShader		_noiseComputeShader;
		ComputeShader		_isoSurfaceVoxelComputeShader;

		Texture3D *			_noiseVolume;

		VkQueue				_asyncComputeQueue;
		uint32_t			_asyncComputeQueueIndex;

		CommandBufferPool	_asyncComputePool;
		VkCommandBuffer		_asyncCommandBuffer;

		ChunkRenderer *		_renderer;

		void	InitializeChunkData(Chunk & newChunk, const glm::ivec3 & position);

	public:
		ChunkLoader(void) = default;
		ChunkLoader(const ChunkLoader&) = delete;
		virtual ~ChunkLoader(void) = default;

		void	Initialize(SwapChain * swapChain, ChunkRenderer * renderer);
		void	GenerateChunk(const glm::ivec3 & position);

		void	Update(const Camera * camera);

		// TODO: method to retrieve the buffer informations

		ChunkLoader &	operator=(ChunkLoader const & src) = delete;
};