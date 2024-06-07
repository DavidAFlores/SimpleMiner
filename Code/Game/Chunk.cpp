#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "ThirdParty/Engine_Code_ThirdParty_Squirrel/SmoothNoise.hpp"

#include "Game/Chunk.hpp"
#include "Game/Game.hpp"

Chunk::Chunk(IntVec2 chunkCoords)
	:m_chunkCoords(chunkCoords)
{
	//m_chunkBounds = AABB3(float(chunkCoords.x * 16), float(chunkCoords.y * 16), 0.f, float((chunkCoords.x + 1) * 16), float((chunkCoords.y + 1) * 16), 128.f);

	AddVertsForAABB3D(m_boundsVertexes, m_chunkBounds, Rgba8(255, 0, 0));

	InitializeBlocks();
}

 

Chunk::~Chunk()
{
	if (m_needsSaving)
	{
		SaveChunk();
	}

	delete m_vbo;
	m_vbo = nullptr;

	delete m_ibo;
	m_ibo = nullptr;

	m_blocks.clear();
	m_boundsVertexes.clear();
}

void Chunk::InitializeBlocks()
{
	//m_blocks.reserve(CHUNK_TOTAL_BLOCKS);
	
	std::vector<uint8_t> buffer;

	bool save = FileReadToBuffer(buffer, GetFile());

	if (save)
	{
		GenerateFromSavedData(buffer);
	}
	else
	{
		GenerateFromPerlin();
	}

	BuildBlockVerts();
}

void Chunk::GenerateFromPerlin()
{
	int	terrainHeightMap[CHUNK_BLOCKS_PER_LAYER];
	int	stoneHeightMap[CHUNK_BLOCKS_PER_LAYER];

	for (int j = 0; j < CHUNK_SIZE_Y; j++)
	{
		for (int i = 0; i < CHUNK_SIZE_X; i++)
		{
			int x = m_chunkCoords.x * CHUNK_SIZE_X + i;
			int y = m_chunkCoords.y * CHUNK_SIZE_Y + j;

			int terrainHeight = 64 + int(30.f * Compute2dPerlinNoise(float(x), float(y), 200.f, 5, 0.5f, 2.0f, 0));
			terrainHeightMap[j * CHUNK_SIZE_Y + i] = terrainHeight;

			int stoneHeight = terrainHeight - int(4.f * Compute2dPerlinNoise(float(x), float(y), 200.f, 5, 0.5f, 2.0f, 0));
			stoneHeightMap[j * CHUNK_SIZE_Y + i] = stoneHeight;
		}
	}

	for (int i = 0; i < CHUNK_TOTAL_BLOCKS; i++)
	{
		Voxel voxel = Voxel(0);

		Vec3 location = GetLocalCoords(i);

		int height = static_cast<int>(location.z);

		int index = static_cast<int>(location.x + location.y * CHUNK_SIZE_X);

		int terrainHeight = terrainHeightMap[index];
		int stoneHeight = stoneHeightMap[index];

		bool fill = true;

		if (height == terrainHeight)
		{
			voxel.m_type = 3;
			fill = false;
		}

		if (height < terrainHeight)
		{
			if (height > stoneHeight)
			{
				voxel.m_type = 2;
				fill = false;
			}

			if (height == stoneHeight)
			{
				if (g_theRandom->RollRandomBool())
				{
					voxel.m_type = 4;
					fill = false;
				}
				else
				{
					voxel.m_type = 2;
					fill = false;
				}
			}

			if (height < stoneHeight)
			{
				voxel.m_type = 4;
				fill = false;

				float random = g_theRandom->RollRandomFloatZeroToOne();

				if (random < .001)
				{
					voxel.m_type = 8;
				}
				else if (random < .005)
				{
					voxel.m_type = 7;
				}
				else if (random < .02)
				{
					voxel.m_type = 6;
				}
				else if (random < .05)
				{
					voxel.m_type = 5;
				}
			}
		}

		if (fill && location.z < CHUNK_SIZE_Z / 2)
		{
			voxel.m_type = 1;
		}

		m_blocks.push_back(voxel);
	}
}

void Chunk::GenerateFromSavedData(std::vector<uint8_t> data)
{
	
	m_blocks.resize(CHUNK_TOTAL_BLOCKS);

	int location = 8;

	int index = 0;

	//int first =		data[0];
	//int second =	data[1];
	//int third =		data[2];
	//int fourth =	data[3];
	//int fifth =		data[4];
	//int sixth =		data[5];
	//int seventh =	data[6];
	//int eighth =	data[7];

	std::string string = "invalid file";

	//GUARANTEE_OR_DIE(first != 'G', string);
	//GUARANTEE_OR_DIE(second != 'C', string);
	//GUARANTEE_OR_DIE(third != 'H', string);
	//GUARANTEE_OR_DIE(fourth != 'K', string);
	//GUARANTEE_OR_DIE(fifth != 1, string);
	//GUARANTEE_OR_DIE(sixth != 4, string);
	//GUARANTEE_OR_DIE(seventh != 4, string);
	//GUARANTEE_OR_DIE(eighth != 7, string);

	while (location < data.size())
	{
		unsigned char type = data[location];

		int amountlocation = location + 1;
		int amount = static_cast<int>(data[amountlocation]);

		for (int i = 0; i < amount; i++)
		{
			int adjustedLocation = i + index;

			m_blocks[adjustedLocation].m_type = type;

			
		}

		index += amount;

		location += 2;
	}
}

void Chunk::BuildBlockVerts()
{
	//int totalTileVerts = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 6 * 4;

	delete m_vbo;
	m_vbo = nullptr;

	delete m_ibo;
	m_ibo = nullptr;

	std::vector<Vertex_PCU>m_vertexes;
	std::vector< unsigned int> m_indexes;

	m_vertexes.reserve(m_blocks.size() * 6 * 4);



	for (int i = 0; i < CHUNK_TOTAL_BLOCKS; i++)
	{
		if (BlockDef::s_blockDefs[m_blocks[i].m_type].m_visible == false)
			continue;

		Vec2 floorUVAtMins, floorUVAtMaxs, wallUVAtMins, wallUVAtMaxs, ceilingUVAtMins, ceilingUVAtMaxs;

		Vec3 location = GetBlockCoords(i);
		Vec3 local = GetLocalCoords(i);
		//bottom

		//if (location.z < 0)
		//{
		//	Voxel* voxel = GetBlockFromLocal(location + Vec3(0.f, 0.f, -1.f));
		//	if (voxel)
		//	{
		//		if (voxel->m_type != 0)
		//		{
		//			SpriteDefinition floorSpriteDef = g_terrainSpriteSheet->GetSpriteDef(BlockDef::s_blockDefs[m_blocks[i].m_type].m_UVsBottom);
		//			floorSpriteDef.GetUVs(floorUVAtMins, floorUVAtMaxs);
		//
		//			AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(1.f, 0.f, 0.f), location, location + Vec3(0.f, 1.f, 0.f), location + Vec3(1.f, 1.f, 0.f), Rgba8::WHITE, AABB2(floorUVAtMins, floorUVAtMaxs));
		//		}
		//	}
		//}

		SpriteDefinition sideSpriteDef = g_terrainSpriteSheet->GetSpriteDef(BlockDef::s_blockDefs[m_blocks[i].m_type].m_UVsSide);

		sideSpriteDef.GetUVs(wallUVAtMins, wallUVAtMaxs);

		//bottom left, bottom right, top right, top left

		//west
		if (local.x == 0)
		{
			AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(0.f, 1.f, 0.f), location, location + Vec3(0.f, 0.f, 1.f), location + Vec3(0.f, 1.f, 1.f), Rgba8(230, 230, 230), AABB2(wallUVAtMins, wallUVAtMaxs));
		}
		else
		{
			if(GetBlockFromLocal(local + Vec3(-1.f, 0.f, 0.f))->m_type == 0)
			{
				AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(0.f, 1.f, 0.f), location, location + Vec3(0.f, 0.f, 1.f), location + Vec3(0.f, 1.f, 1.f), Rgba8(230, 230, 230), AABB2(wallUVAtMins, wallUVAtMaxs));
			}
		}
	
		//north
		if(local.y == CHUNK_SIZE_Y - 1)
		AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(1.f, 1.f, 0.f), location + Vec3(0.f, 1.f, 0.f), location + Vec3(0.f, 1.f, 1.f), location + Vec3(1.f, 1.f, 1.f), Rgba8(200, 200, 200), AABB2(wallUVAtMins, wallUVAtMaxs));
		else
		{
			if (GetBlockFromLocal(local + Vec3(0.f, 1.f, 0.f))->m_type == 0)
			{
				AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(1.f, 1.f, 0.f), location + Vec3(0.f, 1.f, 0.f), location + Vec3(0.f, 1.f, 1.f), location + Vec3(1.f, 1.f, 1.f), Rgba8(200, 200, 200), AABB2(wallUVAtMins, wallUVAtMaxs));
			}
		}
		//east
		if (local.x == CHUNK_SIZE_X - 1)
		AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(1.f, 0.f, 0.f), location + Vec3(1.f, 1.f, 0.f), location + Vec3(1.f, 1.f, 1.f), location + Vec3(1.f, 0.f, 1.f), Rgba8(230, 230, 230), AABB2(wallUVAtMins, wallUVAtMaxs));
		else
		{
			if (GetBlockFromLocal(local + Vec3(1.f, 0.f, 0.f))->m_type == 0)
			{
				AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(1.f, 0.f, 0.f), location + Vec3(1.f, 1.f, 0.f), location + Vec3(1.f, 1.f, 1.f), location + Vec3(1.f, 0.f, 1.f), Rgba8(230, 230, 230), AABB2(wallUVAtMins, wallUVAtMaxs));
			}
		}
		//south
		if(local.y == 0)
		AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(0.f, 0.f, 0.f), location + Vec3(1.f, 0.f, 0.f), location + Vec3(1.f, 0.f, 1.f), location + Vec3(0.f, 0.f, 1.f), Rgba8(200, 200, 200), AABB2(wallUVAtMins, wallUVAtMaxs));
		else
		{
			if (GetBlockFromLocal(local + Vec3(0.f, -1.f, 0.f))->m_type == 0)
			{
				AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(0.f, 0.f, 0.f), location + Vec3(1.f, 0.f, 0.f), location + Vec3(1.f, 0.f, 1.f), location + Vec3(0.f, 0.f, 1.f), Rgba8(200, 200, 200), AABB2(wallUVAtMins, wallUVAtMaxs));
			}
		}
		//top
		if (local.z != CHUNK_SIZE_Z - 1)
		{
			if (GetBlockFromLocal(local + Vec3(0.f, 0.f, 1.f))->m_type == 0)
			{
				SpriteDefinition topSpriteDef = g_terrainSpriteSheet->GetSpriteDef(BlockDef::s_blockDefs[m_blocks[i].m_type].m_UVsTop);
				topSpriteDef.GetUVs(ceilingUVAtMins, ceilingUVAtMaxs);

				AddVertsForQuad3D(m_vertexes, m_indexes, location + Vec3(0.f, 0.f, 1.f), location + Vec3(1.f, 0.f, 1.f), location + Vec3(1.f, 1.f, 1.f), location + Vec3(0.f, 1.f, 1.f), Rgba8::WHITE, AABB2(ceilingUVAtMins, ceilingUVAtMaxs));
			}
		}
	}

	m_numberOfVertexes = int( m_vertexes.size());

	m_vbo = g_theRenderer->CreateVertexBuffer(m_vertexes.size(), VertexType::VERTEX_PCU);
	m_ibo = g_theRenderer->CreateIndexBuffer(m_indexes.size());

	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexes.size() * sizeof(unsigned int), m_ibo);
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_vertexes.size() * sizeof(Vertex_PCU), m_vbo);

	m_isDirty = false;
}

//void Chunk::RebuildChunk()
//{
//	InitializeBlockVerts();
//
//}

void Chunk::Update()
{

}

void Chunk::Render()
{
	//g_theRenderer->SetLightConstants(m_sunDirection.GetNormalized(), m_sunIntensity, m_ambientIntensity);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(g_terrainTexture);
	//g_theRenderer->BindShader(m_mapShader);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	
	g_theRenderer->DrawIndexedBuffer(m_vbo, m_ibo);
}

void Chunk::DebugRender()
{
	//g_theRenderer->SetLightConstants(m_sunDirection.GetNormalized(), m_sunIntensity, m_ambientIntensity);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(g_terrainTexture);
	//g_theRenderer->BindShader(m_mapShader);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);

	g_theRenderer->DrawVertexArray(static_cast<int>(m_boundsVertexes.size()), m_boundsVertexes.data());
}

void Chunk::Shutdown()
{

}

void Chunk::Activate()
{
	m_status = ChunkStatus::CHUNK_STATUS_ACTIVE;
}

void Chunk::Deactivate()
{
	m_status = ChunkStatus::CHUNK_STATUS_INACTIVE;
}

AABB3 Chunk::GetBounds()
{
	return AABB3(float(m_chunkCoords.x * 16), float(m_chunkCoords.y * 16), 0.f, float((m_chunkCoords.x + 1) * 16), float((m_chunkCoords.y + 1) * 16), 128.f);
}

Vec2 Chunk::GetCenter()
{
	float x = float(m_chunkCoords.x * CHUNK_SIZE_X + CHUNK_CENTER_X);
	float y = float(m_chunkCoords.y * CHUNK_SIZE_Y + CHUNK_CENTER_Y);

	return Vec2(x, y);	
}

Vec3 Chunk::GetBlockCoords(int index)
{
	//Vec2 bottomUVs, sideUvs, topUvs;

	float x = (index & CHUNK_MASK_X) + (m_chunkCoords.x * 16.f);
	float y = ((index >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y) + (m_chunkCoords.y * 16.f);
	float z = static_cast<float>(((index >> CHUNK_BITSHIFT_Z) & CHUNK_MASK_Z));

	return Vec3(x, y, z);
}

Vec3 Chunk::GetLocalCoords(int index)
{
	float x = static_cast<float>((index & CHUNK_MASK_X));
	float y = static_cast<float>(((index >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y));
	float z = static_cast<float>(((index >> CHUNK_BITSHIFT_Z) & CHUNK_MASK_Z));

	return Vec3(x, y, z);
}

Voxel* Chunk::GetBlockFromLocal(Vec3 location)
{
	int index = int(location.x + location.y * CHUNK_SIZE_X + location.z * CHUNK_SIZE_X * CHUNK_SIZE_Y);
	return &m_blocks[index];
}

Voxel* Chunk::GetTopBlockFromWorld(Vec2 location)
{
	//int x = m_chunkCoords.x * CHUNK_SIZE_X + int(location.x);
	//int y = m_chunkCoords.y * CHUNK_SIZE_Y + int(location.y);

	int x = RoundDownToInt(location.x) - m_chunkCoords.x * CHUNK_SIZE_X;
															
	int y = RoundDownToInt(location.y) - m_chunkCoords.y * CHUNK_SIZE_Y;

	int height = 127;

	while (height != -1)
	{
		Voxel* block = GetBlockFromLocal(Vec3(float(x), float(y), float(height)));
		if (block->m_type != 0)
		{
			return block;
		}

		height--;
	}
	
	ERROR_AND_DIE("reached bellow world");

	return nullptr;
}

Voxel* Chunk::GetAboveTopBlockFromWorld(Vec2 location)
{
	//int x = m_chunkCoords.x * CHUNK_SIZE_X + int(location.x);
	//int y = m_chunkCoords.y * CHUNK_SIZE_Y + int(location.y);

	int x = RoundDownToInt(location.x) - m_chunkCoords.x * CHUNK_SIZE_X;

	int y = RoundDownToInt(location.y) - m_chunkCoords.y * CHUNK_SIZE_Y;

	int height = 127;

	while (height != -1)
	{
		Voxel* block = GetBlockFromLocal(Vec3(float(x), float(y), float(height)));
		if (block->m_type != 0)
		{
			return GetBlockFromLocal(Vec3(float(x), float(y), float(height + 1)));
		}

		height--;
	}

	ERROR_AND_DIE("reached bellow world");

	return nullptr;
}

void Chunk::ReplaceWithAirBlock(Voxel* voxel)
{
	voxel->m_type = 0;
	m_isDirty = true;
	m_needsSaving = true;
}

void Chunk::ReplaceWithBlock(Voxel* voxel, unsigned char type)
{
	voxel->m_type = type;
	m_isDirty = true;
	m_needsSaving = true;
}

std::string Chunk::GetFile()
{
	std::string string = Stringf("Saves/Chunk(%i, %i).chunk", m_chunkCoords.x, m_chunkCoords.y);

	return string;
}

void Chunk::SaveChunk()
{
	std::vector<uint8_t> buffer;

	buffer.push_back('G');
	buffer.push_back('C');
	buffer.push_back('H');
	buffer.push_back('K');
	buffer.push_back(1);
	buffer.push_back(CHUNK_SIZE_X);
	buffer.push_back(CHUNK_SIZE_Y);
	buffer.push_back(CHUNK_SIZE_Z);

	int currentAmount = 0;
	unsigned char currentType = m_blocks[0].m_type;

	for (int i = 0; i < CHUNK_TOTAL_BLOCKS; i++)
	{
		if (m_blocks[i].m_type != currentType)
		{
			buffer.push_back(currentType);
			buffer.push_back(static_cast<unsigned char>(currentAmount));

			currentType = m_blocks[i].m_type;
			currentAmount = 0;
		}
		currentAmount++;
	}

	std::string name = GetFile();

	FileWriteFromBuffer(buffer, name);
}

//void Chunk::LoadChunk()
//{
//	std::vector<uint8_t> data;
//	FileReadToBuffer(data, )
//}
//

