#include "ChunkRenderer.hpp"
#include "AssetManager.hpp"
#include "VoxelRenderPipeline.hpp"
#include "GUIManager.hpp"

void			ChunkRenderer::Initialize(SwapChain * swapChain)
{
	_swapChain = swapChain;
	_hierarchy = Application::Get()->GetHierarchy();
	const auto & settings = GUIManager::currentSettings;
	
	CreateVoxelVertexDescription();

	// Create material we will use to display the terrain
	_unlitMinecraftMaterial = Material::Create("Shading/UnlitMinecraft.hlsl", "Shading/VoxelVertex.hlsl");
	_unlitMinecraftMaterial->SetVertexInputState(voxelVertexInputStateInfo);
	_unlitMinecraftMaterial->SetTexture(TextureBinding::Albedo, AssetManager::blockAtlas, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	_unlitMinecraftMaterial->SetBuffer("sizeOffsets", AssetManager::blockAtlas->GetSizeOffsetBuffer(), AssetManager::blockAtlas->GetSizeOffsetBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	_unlitMinecraftMaterial->SetBuffer("atlas", AssetManager::blockAtlas->GetAtlasSizeBuffer(), AssetManager::blockAtlas->GetAtlasSizeBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	_renderer = new IndirectRenderer(_unlitMinecraftMaterial);
	_hierarchy->AddGameObject(new GameObject(_renderer));

	// Initialize internal buffers
	_renderer->AllocateDrawBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, swapChain->GetImageCount(), 512); // TODO: hardcoded constant
	_renderer->SetDrawBufferValues(0, 0, 1, 0, 0);
}

void			ChunkRenderer::CreateVoxelVertexDescription(void)
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

void			ChunkRenderer::Render(const Camera * camera, RenderContext * context, RenderPass & pass)
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
			// vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, offsets);

			// We only have one buffer currently so we don't need that
			// indirectRenderer->SetOffset(0 * sizeof(VkDrawIndirectCommand));

			pass.BindDescriptorSet(LWGCBinding::Object, renderer->GetDescriptorSet());

			// We bind / rebind everything we need for the folowing draws
			pass.UpdateDescriptorBindings();

			indirectRenderer->RecordDrawCommand(cmd, 0);
		}
	}
}

VkBuffer		ChunkRenderer::GetDrawBuffer(void) const { size_t unused; printf("R: %p\n", _renderer); return _renderer->GetDrawBuffer(0, unused); }

VkDeviceSize	ChunkRenderer::GetDrawBufferSize(void) const { return _renderer->GetDrawBufferSize(); }