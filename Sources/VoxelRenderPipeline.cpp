#include "Core/Vulkan/VulkanInstance.hpp"
#include "VoxelRenderPipeline.hpp"
#include "Core/Vulkan/Vk.hpp"
#include "Core/Rendering/RenderPipelineManager.hpp"
#include "Core/Vulkan/ProfilingSample.hpp"
#include "LWGC.hpp"
#include "AssetManager.hpp"

using namespace LWGC;

void	VoxelRenderPipeline::Initialize(SwapChain * swapChain)
{
	hierarchy = Application::Get()->GetHierarchy();

	RenderPipeline::Initialize(swapChain);
	chunkRenderer.Initialize(swapChain);
	// We need the loader to have the renderer instance to sync the generated data
	chunkLoader.Initialize(swapChain, &chunkRenderer);

	// Debug cube
	auto debugMaterial = Material::Create(BuiltinShaders::Standard, BuiltinShaders::DefaultVertex);
	debugMaterial->SetTexture(TextureBinding::Albedo, AssetManager::blockAtlas, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	auto cube = new GameObject(new MeshRenderer(PrimitiveType::Cube, debugMaterial));
	cube->GetTransform()->SetPosition(glm::vec3(8, 8, 15));
	hierarchy->AddGameObject(cube);

	// Setup the render passes we uses:
	SetupRenderPasses();
}

void	VoxelRenderPipeline::SetupRenderPasses()
{
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
	Camera *		camera = cameras[0]; // This pipeline don't support multi-camera

	// Generate new chunks if needed for each cameras
	chunkLoader.Update(camera);

	forwardPass.Begin(cmd, GetCurrentFrameBuffer(), "All Cameras");
	{
		forwardPass.BindDescriptorSet(LWGCBinding::Frame, perFrameSet.GetDescriptorSet());
		{
			RenderPipelineManager::beginCameraRendering.Invoke(camera);
			forwardPass.BindDescriptorSet(LWGCBinding::Camera, camera->GetDescriptorSet());

			chunkRenderer.Render(camera, context, forwardPass);

			// Optional: record all mesh renderers so we can see gizmos and debug objects
			RenderPipeline::RecordAllMeshRenderers(forwardPass, context);

			RenderPipelineManager::endCameraRendering.Invoke(camera);
		}
	}
	forwardPass.End();
}
