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
	_unlitMinecraftMaterial->SetBuffer("atlas", AssetManager::blockAtlas->GetAtlasSizeBuffer(), AssetManager::blockAtlas->GetAtlasSizeBufferSize(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	_renderer = new IndirectRenderer(_unlitMinecraftMaterial);
	_hierarchy->AddGameObject(new GameObject(_renderer));

	// Initialize internal buffers
	_renderer->AllocateDrawBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, swapChain->GetImageCount(), 512); // TODO: hardcoded constant
	_renderer->SetDrawBufferValues(0, 0, 1, 0, 0);

	// Initialize chunk description buffer:
	_description.chunkSize = glm::ivec3(settings->chunkSize, settings->chunkSize, settings->chunkSize);
	Vk::CreateBuffer(sizeof(ChunkDescription), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _chunkDescription.buffer, _chunkDescription.memory);
	Vk::UploadToMemory(_chunkDescription.memory, &_description, sizeof(ChunkDescription), 0, true);

	// Bind the chunk description cbuffer:
	_unlitMinecraftMaterial->SetBuffer("description", _chunkDescription.buffer, sizeof(ChunkDescription), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
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
	// TODO: perform occlusion culling here, frustum culling must be done inside the chunkLoader (and passed to this class)

	auto renderQueue = context->GetRenderQueue();
	VkCommandBuffer cmd = pass.GetCommandBuffer();

	// TODO: check if settings have changed and upload the new chunk description

	for (uint32_t i = 0; i < renderQueue->GetQueueCount(); i++)
	{
		const auto & renderers = renderQueue->GetRenderersForQueue(i);

		// Technically, there is only one render (the only we declared in this class)
		// But we keep the loop for future proofing
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

			for (const auto & kp : *_map)
			{
				const auto & chunk = kp.second;
				// Currently all chunks are in separate vertex buffer so we need
				// multiple draw calls to dispay them:

				// When using a drawIndirect, this offset is pretty much useless
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(cmd, 0, 1, &chunk.vertexBuffer.buffer, offsets);

				material->SetPushConstant(cmd, "chunkPosition", &chunk.positionWS);
				material->BindFrameProperties(cmd);
				// std::cout << chunk.positionWS << std::endl;

				// We bind / rebind everything we need for the folowing draws
				pass.UpdateDescriptorBindings();

				// indirectRenderer->SetOffset(chunk.drawBufferIndex);
				indirectRenderer->RecordDrawCommand(cmd, 0);
			}

			// We only have one buffer currently so we don't need that
			// indirectRenderer->SetOffset(0 * sizeof(VkDrawIndirectCommand));

			// TODO: bind the position and not the model matrix
			// pass.BindDescriptorSet(LWGCBinding::Object, renderer->GetDescriptorSet());
		}
	}
}

void			ChunkRenderer::UpdateDrawData(ChunkMap * map)
{
	_map = map;
}

VkBuffer		ChunkRenderer::GetDrawBuffer(void) const { size_t unused; return _renderer->GetDrawBuffer(0, unused); }

VkDeviceSize	ChunkRenderer::GetDrawBufferSize(void) const { return _renderer->GetDrawBufferSize(); }

IndirectRenderer *	ChunkRenderer::GetIndirectRenderer(void) { return _renderer; }