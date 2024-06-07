#include "Game/World.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

World::World(Game* game, Player* player)
	: m_game(game)
	, m_player(player)
{
	
}

World::~World()
{
	for (auto i = m_activeChunks.begin(); i != m_activeChunks.end(); i++)
	{
		delete i->second;
		i->second = nullptr;
	}
	m_activeChunks.clear();
}

void World::Startup()
{
	//m_Chunks.reserve(10000);
	//
	//for (int i = 0; i < 100; i++)
	//{
	//	for (int j = 0; j < 100; j++)
	//	{
	//		m_Chunks.push_back(new Chunk(IntVec2(j - 50, i - 50)));
	//	}
	//}
}

void World::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	ManageChunkSize();

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debug = !m_debug;
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F8))
	{
		DeactivateAllChunks();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		digDownwards();
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		buildDownwards();
	}
}

void World::Render()
{
	for (auto i = m_activeChunks.begin(); i != m_activeChunks.end(); i++)
	{
		i->second->Render();

		if (m_debug)
		{
			i->second->DebugRender();
		}
	}

	//for (int i = 0; i < m_activeChunks.size(); i++)
	//{
	//	m_activeChunks[i]->Render();
	//
	//	if (m_debug)
	//	{
	//		m_activeChunks[i]->DebugRender();
	//	}
	//}
}

void World::EndFrame()
{

	int totalVertexes = 0;

 	for (auto i = m_activeChunks.begin(); i != m_activeChunks.end(); i++)
	{
		totalVertexes += i->second->m_numberOfVertexes;
	}

	std::string string3 = Stringf("Chunks: %d, Vertices: %d", int(m_activeChunks.size()), totalVertexes);

	DebugAddScreenText(string3, Vec2(200.f, 500.f), 200.f, Vec2(1.f, 1.f), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);

	if (m_activeChunks.size() > 10)
	{
		Chunk* chunk = getChunkByPosition(m_player->m_position);

		int x = RoundDownToInt(m_player->m_position.x) - chunk->m_chunkCoords.x * CHUNK_SIZE_X;
		int y = RoundDownToInt(m_player->m_position.y) - chunk->m_chunkCoords.y * CHUNK_SIZE_Y;

		std::string string6 = Stringf("local coord is x: %d, y: %d", x, y);

		DebugAddScreenText(string6, Vec2(200.f, 200.f), 200.f, Vec2(1.f, 1.f), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);

		std::string string5 = Stringf("dug block is x: %d, y: %d", dugblock.x, dugblock.y);

		DebugAddScreenText(string3, Vec2(200.f, 100.f), 200.f, Vec2(1.f, 1.f), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);

		std::string string4 = Stringf("chunk coord is x: %d, y: %d", chunk->m_chunkCoords.x, chunk->m_chunkCoords.y);

		DebugAddScreenText(string4, Vec2(200.f, 000.f), 200.f, Vec2(1.f, 1.f), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);

	}

	if (m_worldEnding)
	{
		m_game->m_theWorld = nullptr;

		m_player = nullptr;

		delete this;
	}
	return;
}

Chunk* World::getChunkByPosition(Vec2 position)
{
	int intx = 0;
	int inty = 0;

	intx = RoundDownToInt(position.x / CHUNK_SIZE_X);
	inty = RoundDownToInt(position.y / CHUNK_SIZE_Y);

	return m_activeChunks.at(IntVec2(intx, inty));
}

//if distance between the chunk center and the player's position is less than chunk activation range
bool World::IsChunkInActivationRange(Chunk* chunk)
{
	float distanceSqrd = (chunk->GetCenter() - m_player->m_position.MakeVec2()).GetLengthSquared();

	float rangeSqrd = CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE;

	if (distanceSqrd < rangeSqrd)
	{
		return true;
	}

	return false;
}

bool World::IsChunkInDeactivationRange(Chunk* chunk)
{
	float distanceSqrd = (chunk->GetCenter() - m_player->m_position.MakeVec2()).GetLengthSquared();

	float rangeSqrd = CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE;

	if (distanceSqrd < rangeSqrd)
	{
		return true;
	}

	return false;
}

void World::ActivateChunk(IntVec2 location, Chunk* chunk)
{
	m_activeChunks.insert({ location, chunk });
	m_activeLocations.push_back(chunk);
	chunk->Activate();

	// ternary example: isMember ? '$2.00' : '$10.00';
	if (m_activeChunks.find(location + IntVec2(0, 1)) != m_activeChunks.end())
	{
		Chunk* northNeighbor = m_activeChunks.at(chunk->m_chunkCoords + IntVec2(0, 1));
		chunk->m_northNeighbor = northNeighbor;

		northNeighbor->m_southNeighbor = chunk;
	}

	if (m_activeChunks.find(location + IntVec2(1, 0)) != m_activeChunks.end())
	{
		Chunk* eastNeighbor = m_activeChunks.at(chunk->m_chunkCoords + IntVec2(1, 0));
		chunk->m_eastNeighbor = eastNeighbor;

		eastNeighbor->m_westNeighbor = chunk;
	}

	if (m_activeChunks.find(location + IntVec2(0, -1)) != m_activeChunks.end())
	{
		Chunk* southNeighbor = m_activeChunks.at(chunk->m_chunkCoords + IntVec2(0, -1));
		chunk->m_southNeighbor = southNeighbor;

		southNeighbor->m_northNeighbor = chunk;
	}

	if (m_activeChunks.find(location + IntVec2(-1, 0)) != m_activeChunks.end())
	{
		Chunk* westNeighbor = m_activeChunks.at(chunk->m_chunkCoords + IntVec2(-1, 0));
		chunk->m_westNeighbor = westNeighbor;

		westNeighbor->m_eastNeighbor = chunk;
	}
}

void World::DeactivateChunk(Chunk* chunk)
{
	if (chunk->m_northNeighbor != nullptr)
	{
		chunk->m_northNeighbor->m_southNeighbor = nullptr;
		chunk->m_northNeighbor = nullptr;
	}

	if (chunk->m_eastNeighbor != nullptr)
	{
		chunk->m_eastNeighbor->m_westNeighbor = nullptr;
		chunk->m_eastNeighbor = nullptr;
	}

	if (chunk->m_southNeighbor != nullptr)
	{
		chunk->m_southNeighbor->m_northNeighbor = nullptr;
		chunk->m_southNeighbor = nullptr;
	}

	if (chunk->m_westNeighbor != nullptr)
	{
		chunk->m_westNeighbor->m_eastNeighbor = nullptr;
		chunk->m_westNeighbor = nullptr;
	}

	m_activeChunks.erase(chunk->m_chunkCoords);

	chunk->Deactivate();

	delete chunk;
}

void World::ManageChunkSize()
{
	if (m_activeChunks.size() == maxChunks)
	{
		DeactivateFarthestChunk();
	}
	else if (m_activeChunks.size() < maxChunks)
	{

		if (ActivateNearestChunk()) {}

		else
		{
			DeactivateFarthestChunk();
		}
	}

	RebuildSingleChunk();
}

void World::DeactivateFarthestChunk()
{
	float longestDistanceSquared = 0.f;
	Chunk* farthestChunk = nullptr;
	Vec2 position = m_player->m_position;

	for (auto i = m_activeChunks.begin(); i != m_activeChunks.end(); i++)
	{
		float distanceSquared = (i->second->GetCenter() - position).GetLengthSquared();
		if (longestDistanceSquared < distanceSquared)
		{
			longestDistanceSquared = distanceSquared;
			farthestChunk = i->second;
		}
	}
	if (farthestChunk != nullptr && longestDistanceSquared > CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE)
	{
		DeactivateChunk(farthestChunk);
	}
	farthestChunk = nullptr;
}

bool World::ActivateNearestChunk()
{
	float shortestDistanceSquared = 99999999999.f;
	//Chunk* closestChunk = nullptr;
	IntVec2 position = IntVec2(int(m_player->m_position.x) / 16, int(m_player->m_position.y) / 16);
	IntVec2 closestChunkLocation = position;

	int xNeighborLength = 32;
	int yNeighborLength = 32;

	//loop 32 * 32 times
	for (int j = 0; j < xNeighborLength; j++)
	{
		for (int i = 0; i < yNeighborLength; i++)
		{
			//chunkcoords = position + intVec2(i, j) - neighborsize
			int xChunkCoord = position.x + i - xNeighborLength / 2;
			int yChunkCoord = position.y + j - yNeighborLength / 2;

			if (m_activeChunks.find(IntVec2(xChunkCoord, yChunkCoord)) == m_activeChunks.end())
			{
				float x = xChunkCoord * CHUNK_SIZE_X + CHUNK_SIZE_X * .5f;
				float y = yChunkCoord * CHUNK_SIZE_Y + CHUNK_SIZE_Y * .5f;

				float distance = (Vec2(x, y) - m_player->m_position.MakeVec2()).GetLengthSquared();
				float distanceSqrd = distance * distance;

				if (shortestDistanceSquared > distanceSqrd)
				{
					shortestDistanceSquared = distanceSqrd;
					closestChunkLocation = IntVec2(xChunkCoord, yChunkCoord);
				}
			}
		}
	}

	Chunk* closestInactiveChunk = new Chunk(closestChunkLocation);

	float distanceSquared = (closestInactiveChunk->GetCenter() - Vec2(m_player->m_position)).GetLengthSquared();
	if (distanceSquared < CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE)
	{
		ActivateChunk(closestChunkLocation, closestInactiveChunk);
		return true;
	}
	else
	{
		delete closestInactiveChunk;
		closestInactiveChunk = nullptr;
		return false;
	}
}

void World::RebuildSingleChunk()
{
	for (auto i = m_activeChunks.begin(); i != m_activeChunks.end(); i++)
	{
		if (i->second->m_isDirty)
		{
			//rebuild
			i->second->BuildBlockVerts();
			return;
		}
	}
}

void World::DeactivateAllChunks()
{
	for (int i = 0; i < m_activeLocations.size(); i++)
	{
		DeactivateChunk(m_activeLocations[i]);
	}

	m_activeLocations.clear();
}

void World::digDownwards()
{
	Vec2 position = m_player->m_position.MakeVec2();

	Chunk* chunk = getChunkByPosition(position);
	Voxel* blockToReplace = chunk->GetTopBlockFromWorld(position);

	chunk->ReplaceWithAirBlock(blockToReplace);
}

void World::buildDownwards()
{
	Vec2 position = m_player->m_position.MakeVec2();

	Chunk* chunk = getChunkByPosition(position);
	Voxel* blockToReplace = chunk->GetAboveTopBlockFromWorld(position);

	chunk->ReplaceWithBlock(blockToReplace, 9);
	
}
