#pragma once

#include <string>

#include "LWGC.hpp"

using namespace LWGC;

class ChunkRenderer
{
	private:
		SwapChain *			_swapChain;

		Material *			_unlitMinecraftMaterial;
		Hierarchy *			_hierarchy;
		IndirectRenderer *	_renderer;

		VkPipelineVertexInputStateCreateInfo	voxelVertexInputStateInfo;

		void	CreateVoxelVertexDescription(void);

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