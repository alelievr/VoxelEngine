#include "Core/Vulkan/VulkanInstance.hpp"
#include "Core/Rendering/VoxelRenderPipeline.hpp"
#include "Core/Vulkan/Vk.hpp"
#include "Core/Rendering/RenderPipelineManager.hpp"
#include "Core/Vulkan/ProfilingSample.hpp"

using namespace LWGC;

Texture2D * fractalTexture;

void	VoxelRenderPipeline::Initialize(SwapChain * swapChain)
{
	RenderPipeline::Initialize(swapChain);

	heavyComputeShader.LoadShader("Shaders/Compute/Heavy.hlsl");
	heavyComputeFence = Vk::CreateFence(true);

	// Allocate an async command queue (the device must have more than one queue to run the application)
	instance->AllocateDeviceQueue(asyncComputeQueue, asyncComputeQueueIndex);
	asyncComputePool.Initialize(asyncComputeQueue, asyncComputeQueueIndex);
	asyncCommandBuffer = asyncComputePool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	fractalTexture = Texture2D::Create(2048, 2048, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	heavyComputeShader.SetTexture("fractal", fractalTexture, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	heavyComputeShader.SetBuffer(LWGCBinding::Frame, _uniformPerFrame.buffer, sizeof(LWGC_PerFrame), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = fractalTexture->GetImage();

	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	heavyComputeShader.AddImageBarrier(barrier, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	// By default the descriptor set is created with stage all flags
	asyncComputeSet.AddBinding(0, fractalTexture, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	// Test
	if (VulkanInstance::IsRayTracingEnabled())
	{
		VkBuffer aabbBuffer;
		VkDeviceMemory mem;
		Vk::CreateBuffer(10, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, aabbBuffer, mem);

		VkGeometryAABBNV aabb = {};
		aabb.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		aabb.aabbData = aabbBuffer;
		aabb.numAABBs = 1;
		aabb.stride = 8;
		aabb.offset = 0;

		VkGeometryDataNV data = {};
		data.aabbs = aabb;

		VkGeometryNV	geom = {};
		geom.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		geom.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
		geom.geometry = data;
		geom.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

		VkAccelerationStructureInfoNV accelInfo = {};
		accelInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		accelInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
		accelInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV;
		accelInfo.instanceCount = 1;
		accelInfo.geometryCount = 1;
		accelInfo.pGeometries = &geom;

		VkAccelerationStructureCreateInfoNV info = {};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		info.compactedSize = 10;
		info.info = accelInfo;

		VkAccelerationStructureNV	structure;
		vkCreateAccelerationStructureNV(device, &info, nullptr, &structure);
	}

	// Setup the render passes we uses:
	SetupRenderPasses();
}

void	VoxelRenderPipeline::SetupRenderPasses()
{
	// Compute pass (for each frame)
	computePass.Initialize(swapChain);
	// For compute we don't need to call Create on renderpass because we don't need one to run a compute shader

	// Forward pass (render all objects into the framebuffer)
	forwardPass.Initialize(swapChain);

	forwardPass.AddAttachment(
		RenderPass::GetDefaultColorAttachment(swapChain->GetImageFormat()),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	forwardPass.SetDepthAttachment(
		RenderPass::GetDefaultDepthAttachment(instance->FindDepthFormat()),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	);

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // useful ?

	forwardPass.AddDependency(dependency);

	forwardPass.SetClearColor(Color::Black, 1.0f, 0.0f);

	forwardPass.Create();

	// Must be the last renderpass before the final blit (which contains the framebuffer attachements)
	SetLastRenderPass(forwardPass);
}

void	VoxelRenderPipeline::Render(const std::vector< Camera * > & cameras, RenderContext * context)
{
	VkCommandBuffer cmd = GetCurrentFrameCommandBuffer();

	{
		auto computeSample = ProfilingSample("Noise Dispatch");

		auto asyncCmd = asyncComputePool.BeginSingle();
		heavyComputeShader.Dispatch(cmd, 4096, 4096, 1);
		asyncComputePool.EndSingle(asyncCmd); // fence
	}

	// Process the compute shader before everything:
	computePass.Begin(cmd, VK_NULL_HANDLE, "All Computes");
	{
		computePass.BindDescriptorSet(LWGCBinding::Frame, perFrameSet.GetDescriptorSet());
		RenderPipeline::RecordAllComputeDispatches(computePass, context);
	}
	computePass.End();

	forwardPass.Begin(cmd, GetCurrentFrameBuffer(), "All Cameras");
	{
		forwardPass.BindDescriptorSet(LWGCBinding::Frame, perFrameSet.GetDescriptorSet());
		forwardPass.BindDescriptorSet("asyncTexture", asyncComputeSet);
		for (const auto camera : cameras)
		{
			RenderPipelineManager::beginCameraRendering.Invoke(camera);
			forwardPass.BindDescriptorSet(LWGCBinding::Camera, camera->GetDescriptorSet());

			RenderPipeline::RecordAllMeshRenderers(forwardPass, context);

			RenderPipelineManager::endCameraRendering.Invoke(camera);
		}
	}
	forwardPass.End();
}