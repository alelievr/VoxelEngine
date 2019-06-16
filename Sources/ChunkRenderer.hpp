#pragma once

#include <string>

#include "LWGC.hpp"
#include "ChunkMap.hpp"

class ChunkLoader;

using namespace LWGC;

class ChunkRenderer
{
	friend class ChunkLoader;

	private:
		SwapChain *			_swapChain;

		Material *			_unlitMinecraftMaterial;
		Hierarchy *			_hierarchy;
		IndirectRenderer *	_renderer;
		ChunkMap *			_map;

		VkPipelineVertexInputStateCreateInfo	voxelVertexInputStateInfo;

		void	CreateVoxelVertexDescription(void);
		void	UpdateDrawData(ChunkMap * map);

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