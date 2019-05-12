#include "Core/Vulkan/VulkanInstance.hpp"
#include "VoxelRenderPipeline.hpp"
#include "Core/Vulkan/Vk.hpp"
#include "Core/Rendering/RenderPipelineManager.hpp"
#include "Core/Vulkan/ProfilingSample.hpp"
#include "LWGC.hpp"

using namespace LWGC;

void	VoxelRenderPipeline::Initialize(SwapChain * swapChain)
{
	RenderPipeline::Initialize(swapChain);

	hierarchy = Application::Get()->GetHierarchy();

	noiseComputeShader.LoadShader("Noises/Spheres.hlsl");
	isoSurfaceVoxelComputeShader.LoadShader("Meshing/Voxels.hlsl");

	unlitMinecraftMaterial = Material::Create("Shading/UnlitMinecraft.hlsl", "Shading/VoxelVertex.hlsl");

	// We start with chunks of
	noiseVolume = Texture3D::Create(128, 128, 128, VK_FORMAT_R8_SNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	noiseVolume->ChangeLayout(VK_IMAGE_LAYOUT_GENERAL);

	noiseComputeShader.SetTexture("noiseVolume", noiseVolume, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	// TODO: is it worth to make an image layout transition between these two calls ?
	isoSurfaceVoxelComputeShader.SetTexture("noiseVolume", noiseVolume, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	CreateVertexDescription();

	// Allocate the vertex buffer:
	Vk::CreateBuffer(sizeof(VoxelVertexAttributes) * 128 * 128 * 64 * 6, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexMemory);

	isoSurfaceVoxelComputeShader.SetBuffer("vertices", vertexBuffer, sizeof(VoxelVertexAttributes) * 128 * 128 * 64 * 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	// Debug cube
	auto debugMaterial = Material::Create(BuiltinShaders::ColorDirection, BuiltinShaders::DefaultVertex);
	auto cube = new GameObject(new MeshRenderer(PrimitiveType::Cube, debugMaterial));
	cube->GetTransform()->SetPosition(glm::vec3(0, 0, 5));
	hierarchy->AddGameObject(cube);

	unlitMinecraftMaterial->SetVertexInputState(voxelVertexInputStateInfo);

	renderer = new IndirectRenderer(unlitMinecraftMaterial);
	hierarchy->AddGameObject(new GameObject(renderer));

	renderer->AllocateDrawBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, swapChain->GetImageCount(), 1);

	// Allocate an async command queue (the device must have more than one queue to run the application)
	instance->AllocateDeviceQueue(asyncComputeQueue, asyncComputeQueueIndex);
	asyncComputePool.Initialize(asyncComputeQueue, asyncComputeQueueIndex);
	asyncCommandBuffer = asyncComputePool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	// heavyComputeFence = Vk::CreateFence(true);

	// fractalTexture = Texture2D::Create(2048, 2048, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	// noiseComputeShader.SetTexture("fractal", fractalTexture, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	// noiseComputeShader.SetBuffer(LWGCBinding::Frame, _uniformPerFrame.buffer, sizeof(LWGC_PerFrame), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// VkImageMemoryBarrier barrier = {};
	// barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    // barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    // barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.image = fractalTexture->GetImage();

	// barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // barrier.subresourceRange.baseMipLevel = 0;
    // barrier.subresourceRange.levelCount = 1;
    // barrier.subresourceRange.baseArrayLayer = 0;
    // barrier.subresourceRange.layerCount = 1;

	// barrier.srcAccessMask = 0;
	// barrier.dstAccessMask = 0;

	// noiseComputeShader.AddImageBarrier(barrier, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	// // By default the descriptor set is created with stage all flags
	// asyncComputeSet.AddBinding(0, fractalTexture, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	// Setup the render passes we uses:
	SetupRenderPasses();
}

void	VoxelRenderPipeline::CreateVertexDescription(void)
{
	// Create indirect draw vertex description
	// TODO: refactor by removing statics
	static std::array< VkVertexInputBindingDescription, 1 > bindingDescription = {};
	bindingDescription[0].binding = 0;
	bindingDescription[0].stride = sizeof(VoxelVertexAttributes);
	bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	static std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VoxelVertexAttributes, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VoxelVertexAttributes, atlasIndex);

	voxelVertexInputStateInfo = {};
	voxelVertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	voxelVertexInputStateInfo.vertexBindingDescriptionCount = bindingDescription.size();
	voxelVertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	voxelVertexInputStateInfo.pVertexBindingDescriptions = bindingDescription.data();
	voxelVertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
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

	forwardPass.SetClearColor(Color::Black, 0.0f, 0.0f);

	forwardPass.Create();

	// Must be the last renderpass before the final blit (which contains the framebuffer attachments)
	SetLastRenderPass(forwardPass);
}

void	VoxelRenderPipeline::Render(const std::vector< Camera * > & cameras, RenderContext * context)
{
	VkCommandBuffer cmd = GetCurrentFrameCommandBuffer();
	
	// First, before the generation, we reset the draw buffer
	renderer->SetDrawBufferValues(currentFrame, 0, 1, 0, 0);

	{
		const auto & computeSample = ProfilingSample("Noise Generation");

		auto computeCmd = asyncComputePool.BeginSingle();
		noiseComputeShader.Dispatch(cmd, 128, 128, 128);
		asyncComputePool.EndSingle(computeCmd); // TODO: unneeded fence
	}

	{
		const auto & computeSample = ProfilingSample("Geometry Generation");

		auto asyncCmd = asyncComputePool.BeginSingle();

		// Retrieve the buffer offset to write the draw arguments from the GPU iso-surface algorithm
		size_t bufferIndex;
		VkBuffer drawBuffer = renderer->GetDrawBuffer(currentFrame, bufferIndex);
		// Reset the vertex number before the generation:
		// TODO: rename this
		isoSurfaceVoxelComputeShader.SetBuffer("drawCommands", drawBuffer, renderer->GetDrawBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		// Update the draw index via push-constants:
		isoSurfaceVoxelComputeShader.SetPushConstant(cmd, "targetDrawIndex", &bufferIndex);
		isoSurfaceVoxelComputeShader.Dispatch(cmd, 8, 8, 8); // small dispatch to test
		asyncComputePool.EndSingle(asyncCmd); // fence
	}

	// TODO: readback counter to have the number of generated vertices

	forwardPass.Begin(cmd, GetCurrentFrameBuffer(), "All Cameras");
	{
		forwardPass.BindDescriptorSet(LWGCBinding::Frame, perFrameSet.GetDescriptorSet());
		forwardPass.BindDescriptorSet("asyncTexture", asyncComputeSet);
		for (const auto camera : cameras)
		{
			RenderPipelineManager::beginCameraRendering.Invoke(camera);
			forwardPass.BindDescriptorSet(LWGCBinding::Camera, camera->GetDescriptorSet());

			RecordIndirectDraws(forwardPass, context);

			RenderPipelineManager::endCameraRendering.Invoke(camera);
		}
	}
	forwardPass.End();
}

void	VoxelRenderPipeline::RecordIndirectDraws(RenderPass & pass, RenderContext * context)
{
	auto renderQueue = context->GetRenderQueue();
	VkCommandBuffer cmd = pass.GetCommandBuffer();

	for (uint32_t i = 0; i < renderQueue->GetQueueCount(); i++)
	{
		const auto & renderers = renderQueue->GetRenderersForQueue(i);

		for (auto renderer : renderers)
		{
			// we only care about the terrain here
			auto indirectRenderer = dynamic_cast< IndirectRenderer * >(renderer);
			if (indirectRenderer == nullptr)
				continue ;

			auto material = renderer->GetMaterial();
			pass.BindMaterial(material);

			// TODO: optimize this when doing the renderqueues (sort materials and avoid pipeline switches)
			material->BindPipeline(cmd);
			material->BindProperties(cmd);

			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, offsets);

			// We only have one buffer currently so we don't need that
			// indirectRenderer->SetOffset(currentFrame * sizeof(VkDrawIndirectCommand));

			// indirectRenderer->SetDrawBufferValues(currentFrame, 512, 1, 0, 0);

			pass.BindDescriptorSet(LWGCBinding::Object, renderer->GetDescriptorSet());

			// We bind / rebind everything we need for the folowing draws
			pass.UpdateDescriptorBindings();

			indirectRenderer->RecordDrawCommand(cmd, currentFrame);
		}
	}

	// Optional: record all mesh renderers so we can see gizmos and debug objects 
	RenderPipeline::RecordAllMeshRenderers(forwardPass, context);
}