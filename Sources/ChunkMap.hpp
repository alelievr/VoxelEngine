#pragma once

#include <unordered_map>
#include "LWGC.hpp"

#include "IncludeDeps.hpp"
#include GLM_INCLUDE

struct Chunk
{
	UniformBuffer	vertexBuffer;
	VkDeviceSize	vertexBufferSize;
	int				lod;
	int				drawBufferIndex;
	glm::ivec3		positionWS;
	// TODO: chunk readback datas
};

struct PositionHash
{
	// The idea of this hash function is to collide only when two position are really far away, since
	// we have a max chunk load distance that is very small compared to the hash collision rate, we can
	// be sure to avoid to have two position with the same hash within a big range
	std::size_t operator () (const glm::ivec3 & position) const
	{
		return (position.x * 18397) + (position.y * 20483) + (position.z * 29303);
	}
};

using ChunkMap = std::unordered_map< glm::ivec3, Chunk, PositionHash >;
