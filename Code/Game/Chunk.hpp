#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Game/Voxel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class VertexBuffer;
class IndexBuffer;

//15
constexpr int CHUNK_BITS_X = 4;
//15
constexpr int CHUNK_BITS_Y = 4;
//127
constexpr int CHUNK_BITS_Z = 7;

//16
constexpr int CHUNK_SIZE_X = 1 << CHUNK_BITS_X;

//16
constexpr int CHUNK_SIZE_Y = 1 << CHUNK_BITS_Y;

//128
constexpr int CHUNK_SIZE_Z = 1 << CHUNK_BITS_Z;

//1111
constexpr int CHUNK_MASK_X = CHUNK_SIZE_X - 1;
//1111
constexpr int CHUNK_MASK_Y = CHUNK_SIZE_Y - 1;
//11111111
constexpr int CHUNK_MASK_Z = CHUNK_SIZE_Z - 1;

//number of positions to shift
constexpr int CHUNK_BITSHIFT_X = 0;
constexpr int CHUNK_BITSHIFT_Y = CHUNK_BITS_X;
constexpr int CHUNK_BITSHIFT_Z = CHUNK_BITS_X + CHUNK_BITS_Y;

//256
constexpr int CHUNK_BLOCKS_PER_LAYER = (CHUNK_SIZE_X * CHUNK_SIZE_Y);
//32768
constexpr int CHUNK_TOTAL_BLOCKS = (CHUNK_BLOCKS_PER_LAYER * CHUNK_SIZE_Z);

constexpr float CHUNK_ACTIVATION_RANGE = 250.f;
constexpr float CHUNK_DEACTIVATION_RANGE = CHUNK_ACTIVATION_RANGE + CHUNK_SIZE_X + CHUNK_SIZE_Y;

constexpr int CHUNK_CENTER_X = CHUNK_SIZE_X / 2;
constexpr int CHUNK_CENTER_Y = CHUNK_SIZE_Y / 2;

//constexpr IntVec2 CHUNK_CENTER_XY(CHUNK_CENTER_X, CHUNK_CENTER_Y);

enum class ChunkStatus
{
	CHUNK_STATUS_ACTIVE,
	CHUNK_STATUS_INACTIVE,
	COUNT
};

class Chunk
{
public:
	Chunk(IntVec2 chunkCoords);

	~Chunk();

	void InitializeBlocks();
	void GenerateFromPerlin();
	void GenerateFromSavedData(std::vector<uint8_t> data);
	void BuildBlockVerts();
	//void RebuildChunk();

	void Update();
	void Render();
	void DebugRender();

	void Shutdown();

	void Activate();
	void Deactivate();

	AABB3 GetBounds();
	Vec2 GetCenter();

	Vec3 GetBlockCoords(int index);
	Vec3 GetLocalCoords(int index);

	Voxel* GetBlockFromLocal(Vec3 location);
	Voxel* GetTopBlockFromWorld(Vec2 location);
	Voxel* GetAboveTopBlockFromWorld(Vec2 location);

	void ReplaceWithAirBlock(Voxel* voxel);
	void ReplaceWithBlock(Voxel* voxel, unsigned char type);
	//void ReplaceWithNonAirBlock(Voxel* voxel);

	std::string GetFile();

	void SaveChunk();
	//void LoadChunk();

public:
	IntVec2 m_chunkCoords = IntVec2(0, 0);

	Chunk* m_northNeighbor = nullptr;
	Chunk* m_eastNeighbor = nullptr;
	Chunk* m_southNeighbor = nullptr;
	Chunk* m_westNeighbor = nullptr;

	bool m_isDirty = true;
	bool m_needsSaving = false;

	int m_numberOfVertexes = 0;

private:
	ChunkStatus m_status = ChunkStatus::COUNT;
	AABB3 m_chunkBounds = AABB3();
	std::vector<Voxel> m_blocks;
	
	std::vector<Vertex_PCU>m_boundsVertexes;
	

	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;

	
};



//int index = x | (y << CHUNK_BITSHIFT_Y) | (z << CHUNK_BITSHIFT_Z)