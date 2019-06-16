#pragma once

#include <string>

#include "LWGC.hpp"
#include "TerrainSettings.hpp"
#include <unordered_map>

#include "IncludeDeps.hpp"
#include GLM_INCLUDE

using namespace LWGC;

struct Chunk
{
	UniformBuffer	vertexBuffer;
	VkDeviceSize	vertexBufferSize;
	int				lod;
	int				drawBufferIndex;
	glm::ivec3		positionWS;
	// TODO: chunk readback datas
};

struct PositionHash
{
	// The idea of this hash function is to collide only when two position are really far away, since
	// we have a max chunk load distance that is very small compared to the hash collision rate, we can
	// be sure to avoid to have two position with the same hash within a big range
	std::size_t operator () (const glm::ivec3 & position) const
	{
		return (position.x * 18397) + (position.y * 20483) + (position.z * 29303);
	}
};

class ChunkLoader
{
	private:
		using ChunkMap = std::unordered_map< glm::ivec3, Chunk, PositionHash >;

		RenderPass			computePass; // Used to dispatch the compute shaders

		SwapChain *			_swapChain;
		ChunkMap			_loadedChunks;

		ComputeShader		_noiseComputeShader;
		ComputeShader		_isoSurfaceVoxelComputeShader;

		Texture3D *			_noiseVolume;

		VkQueue				_asyncComputeQueue;
		uint32_t			_asyncComputeQueueIndex;

		CommandBufferPool	_asyncComputePool;
		VkCommandBuffer		_asyncCommandBuffer;

		VkBuffer			_drawBuffer;

		void	InitializeChunkData(Chunk & newChunk, const glm::ivec3 & position);

	public:
		ChunkLoader(void) = default;
		ChunkLoader(const ChunkLoader&) = delete;
		virtual ~ChunkLoader(void) = default;

		void	Initialize(SwapChain * swapChain, VkBuffer drawBuffer, VkDeviceSize drawBufferSize);
		void	GenerateChunk(const glm::ivec3 & position);

		void	Update(const Camera * camera);

		// TODO: method to retrieve the buffer informations

		ChunkLoader &	operator=(ChunkLoader const & src) = delete;
};