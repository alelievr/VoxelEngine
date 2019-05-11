#pragma once

#include <iostream>
#include <string>

#include "Core/Rendering/RenderPipeline.hpp"
#include "Core/Shaders/ComputeShader.hpp"
#include "Core/Vulkan/DescriptorSet.hpp"
#include "Core/Hierarchy.hpp"
#include "Core/Components/IndirectRenderer.hpp"
#include "Core/Textures/Texture3D.hpp"
#include "Core/Application.hpp"

namespace LWGC
{
	struct VoxelVertexAttributes
	{
		glm::vec3		position;
		uint32_t		atlasIndex;
	};

	class		VoxelRenderPipeline : public RenderPipeline
	{
		private:
			RenderPass			computePass; // Dispatch the compute shaders
			RenderPass			forwardPass; // Render all objects in forward

			VkQueue				asyncComputeQueue;
			uint32_t			asyncComputeQueueIndex;

			CommandBufferPool	asyncComputePool;
			VkCommandBuffer		asyncCommandBuffer;

			ComputeShader		noiseComputeShader;
			ComputeShader		isoSurfaceVoxelComputeShader;

			Texture3D *			noiseVolume;

			VkBuffer			vertexBuffer;
			VkDeviceMemory		vertexMemory;

			VkPipelineVertexInputStateCreateInfo	voxelVertexInputStateInfo;

			Material *			unlitMinecraftMaterial;
			Hierarchy *			hierarchy;
			IndirectRenderer *	renderer;

			DescriptorSet		asyncComputeSet;

			void	SetupRenderPasses(void);
			void	CreateVertexDescription(void);
			void	RecordIndirectDraws(RenderPass & pass, RenderContext * context);

		protected:
			void	Render(const std::vector< Camera * > & cameras, RenderContext * context) override;
			void	Initialize(SwapChain * swapChain) override;

		public:
			VoxelRenderPipeline(void) = default;
			virtual ~VoxelRenderPipeline(void) = default;
	};
}
