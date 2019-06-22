#include "ChunkLoader.hpp"
#include "GUIManager.hpp"
#include "VoxelRenderPipeline.hpp"

void	ChunkLoader::Initialize(SwapChain * swapChain, ChunkRenderer * renderer)
{
	const auto & settings = GUIManager::currentSettings;
	_swapChain = swapChain;
	_renderer = renderer;
	VulkanInstance * instance = VulkanInstance::Get();

	_noiseComputeShader.LoadShader("Noises/Spheres.hlsl");
	// Load the compute shader used to generate the noise and the geometry
	_isoSurfaceVoxelComputeShader.LoadShader("Meshing/Voxels.hlsl");

	// Allocate texture 3D to store the noise:
	_noiseVolume = Texture3D::Create(128, 128, 128, VK_FORMAT_R8_SNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	// TODO: test perf using dynamic layout switch between VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL and VK_IMAGE_LAYOUT_GENERAL
	_noiseVolume->ChangeLayout(VK_IMAGE_LAYOUT_GENERAL);

	// Bind compute params
	_noiseComputeShader.SetTexture("noiseVolume", _noiseVolume, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	_isoSurfaceVoxelComputeShader.SetTexture("noiseVolume", _noiseVolume, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	_isoSurfaceVoxelComputeShader.SetBuffer("drawCommands", _renderer->GetDrawBuffer(), _renderer->GetDrawBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	// TODO: upload the terrain settings parameters to the noise

	// Allocate resources for the compute queue:
	instance->AllocateDeviceQueue(_asyncComputeQueue, _asyncComputeQueueIndex);
	_asyncComputePool.Initialize(_asyncComputeQueue, _asyncComputeQueueIndex);
	_asyncCommandBuffer = _asyncComputePool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void	ChunkLoader::InitializeChunkData(Chunk & newChunk, const glm::ivec3 & position)
{
	// TODO: Allocate a big chunk of memory to hold multiple vertex buffers, then defragment the memory into this buffer
	// The purpose is to fill holes into the vertex buffer, it is required to do this
	// because there is a lot of differences between the common and the worst case in term of
	// memory consumption.

	// TODO: be careful of the GPU out of memory !

	// Allocate the vertex buffer: (TODO: reuse a temp vertex buffer)
	newChunk.vertexBufferSize = sizeof(VoxelVertexAttributes) * 128 * 128 * 128 * 2; // Allocate way less memory for the GPU to not stall
	Vk::CreateBuffer(
		newChunk.vertexBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		newChunk.vertexBuffer.buffer,
		newChunk.vertexBuffer.memory
	);

	newChunk.positionWS = position;
	newChunk.lod = 0; // We don't suport LODs yet
	newChunk.drawBufferIndex = -1;
}

int		ChunkLoader::GetEmptyDrawIndexSlot(void)
{
	static int id = 0;

	// TODO: a system to manage chunk rendering slots
	return id++ % 512;
}

void	ChunkLoader::GenerateChunk(const glm::ivec3 & position)
{
	Chunk	newChunk;

	if (_loadedChunks.find(position) != _loadedChunks.end())
		newChunk = _loadedChunks[position];
	else
	{
		InitializeChunkData(newChunk, position);
	}

	{
		ProfilingSample computeSample("Noise Generation");

		auto computeCmd = _asyncComputePool.BeginSingle();
		_noiseComputeShader.BindFrameProperties(computeCmd);
		_noiseComputeShader.Dispatch(computeCmd, 128, 128, 128);
		_asyncComputePool.EndSingle(computeCmd); // TODO: unneeded fence

		computeSample.End();
	}

	{
		ProfilingSample computeSample("Geometry Generation");

		auto asyncCmd = _asyncComputePool.BeginSingle();

		_isoSurfaceVoxelComputeShader.SetBuffer("vertices", newChunk.vertexBuffer.buffer, sizeof(VoxelVertexAttributes) * 128 * 128 * 64 * 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		// Retrieve the buffer offset to write the draw arguments from the GPU iso-surface algorithm
		newChunk.drawBufferIndex = GetEmptyDrawIndexSlot(); // TODO
		// _isoSurfaceVoxelComputeShader.SetBuffer("drawCommands", drawBuffer, renderer->GetDrawBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		_renderer->GetIndirectRenderer()->SetDrawBufferValues(0, newChunk.drawBufferIndex, 0, 1, 0, 0);
		// Update the draw index via push-constants:
		_isoSurfaceVoxelComputeShader.SetPushConstant(asyncCmd, "targetDrawIndex", &newChunk.drawBufferIndex);
		_isoSurfaceVoxelComputeShader.Dispatch(asyncCmd, 128, 128, 128); // small dispatch to test
		_asyncComputePool.EndSingle(asyncCmd); // fence

		computeSample.End();
	}

	// TODO: readback counter to have the number of generated vertices

	_loadedChunks[position] = newChunk;

	std::cout << "Chunk generated at " << position << std::endl;
}

void	ChunkLoader::Update(const Camera * camera)
{
	// TODO: go full async here ans see if we can win some perf
	// TODO: try to enabled async during the rendering too

	glm::ivec3 chunkPosition = glm::floor(camera->GetTransform()->GetPosition() / 128.0f);

	if (_loadedChunks.find(chunkPosition) == _loadedChunks.end())
	{
		GenerateChunk(chunkPosition);
	}

	// Sync generated data with the renderer
	_renderer->UpdateDrawData(&_loadedChunks);
}

// Fence code for async:
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
