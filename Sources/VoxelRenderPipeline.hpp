#pragma once

#include <iostream>
#include <string>

#include "RenderPipeline.hpp"
#include "Core/Shaders/ComputeShader.hpp"
#include "Core/Vulkan/DescriptorSet.hpp"

namespace LWGC
{
	class		VoxelRenderPipeline : public RenderPipeline
	{
		private:
			RenderPass			computePass; // Dispatch the compute shaders
			RenderPass			forwardPass; // Render all objects in forward

			VkQueue				asyncComputeQueue;
			uint32_t			asyncComputeQueueIndex;

			CommandBufferPool	asyncComputePool;
			VkCommandBuffer		asyncCommandBuffer;

			ComputeShader		heavyComputeShader;
			VkFence				heavyComputeFence;

			DescriptorSet		asyncComputeSet;

			void	SetupRenderPasses(void);

		protected:
			void	Render(const std::vector< Camera * > & cameras, RenderContext * context) override;
			void	Initialize(SwapChain * swapChain) override;

		public:
			VoxelRenderPipeline(void) = default;
			virtual ~VoxelRenderPipeline(void) = default;
	};
}
