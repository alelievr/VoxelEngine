#pragma once

#include <string>

#include "LWGC.hpp"
#include "ChunkMap.hpp"

class ChunkLoader;

using namespace LWGC;

struct ChunkDescription
{
	glm::ivec3 chunkSize;
};

class ChunkRenderer
{
	friend class ChunkLoader;

	private:
		SwapChain *			_swapChain;

		Material *			_unlitMinecraftMaterial;
		Hierarchy *			_hierarchy;
		IndirectRenderer *	_renderer;
		ChunkMap *			_map;
		UniformBuffer		_chunkDescription;
		ChunkDescription	_description;

		VkPipelineVertexInputStateCreateInfo	voxelVertexInputStateInfo;

		void	CreateVoxelVertexDescription(void);
		void	UpdateDrawData(ChunkMap * map);
		IndirectRenderer *	GetIndirectRenderer(void);

	public:
		ChunkRenderer(void) = default;
		ChunkRenderer(const ChunkRenderer&) = delete;
		virtual ~ChunkRenderer(void) = default;

		void			Initialize(SwapChain * swapChain);
		void			Render(const Camera * camera, RenderContext * context, RenderPass & pass);
		VkBuffer		GetDrawBuffer(void) const;
		VkDeviceSize	GetDrawBufferSize(void) const;

		ChunkRenderer &	operator=(ChunkRenderer const & src) = delete;
};