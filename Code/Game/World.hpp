#pragma once

//#include "Game/Entity.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Chunk.hpp"
//#include "Engine/Core/HeatMaps.hpp"

#include <vector>
#include <map>


constexpr int NUM_TILE_TRIS = 2;

constexpr int NUM_TILE_VERTS = 3 * NUM_TILE_TRIS;

constexpr int maxChunksRadiusX = 1 + int(CHUNK_ACTIVATION_RANGE) / CHUNK_SIZE_X;
constexpr int maxChunksRadiusY = 1 + int(CHUNK_ACTIVATION_RANGE) / CHUNK_SIZE_Y;
constexpr int maxChunks = (2 * maxChunksRadiusX) * (2 * maxChunksRadiusY);

//class VertexBuffer;
//class IndexBuffer;
class Shader;
class Actor;
//struct ActorUID;
class Game;
class Player;

//struct SpawnInfo
//{
//	std::string m_name = "";
//
//	std::string m_actor = "";
//	Vec3 m_position = Vec3();
//	EulerAngles m_orientation = EulerAngles();
//	Vec3 m_velocity = Vec3();
//};

class World
{
public:
	World(Game* game, Player* player);
	~World();

	void Startup();

	void BeginFrame();
	void Update(float deltaSeconds);
	void Render();
	void ScreenRender();
	void DebugRender();
	void EndFrame();

	Chunk* getChunkByPosition(Vec2 position);

	bool IsChunkInActivationRange(Chunk* chunk);
	bool IsChunkInDeactivationRange(Chunk* chunk);

	void ActivateChunk(IntVec2 location, Chunk* chunk);
	void DeactivateChunk(Chunk* chunk);

	void ManageChunkSize();
	void DeactivateFarthestChunk();
	bool ActivateNearestChunk();
	void RebuildSingleChunk();

	void DeactivateAllChunks();

	void digDownwards();
	void buildDownwards();

	void PushEntitiesOutOfTiles();
	void PushEntitiesOutOfEachOther();

	//void PushEntityOutOfTiles(Entity* entity);
	//void PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 tileCoords);

public:
	Game* m_game;
	Player* m_player;

	//int m_enemyCount = 0;
	float m_mapTimer;

	bool m_worldEnding = false;

	//bool m_playerActorIsDestroyed = false;

	//const MapDefinition* m_definition;

	std::string m_mapImageName;

	std::string m_mapShaderName;

	//Shader* m_mapShader;

	//SpriteSheet* terrainSpriteSheet = nullptr;

	//std::vector< Tile > m_tiles; // Note: this is NOT a 2D array!

	//std::vector<Chunk*> m_Chunks;
	std::map<IntVec2, Chunk*> m_activeChunks;
	std::vector<Chunk*> m_activeLocations;

	std::vector<Actor*> m_allActors;

private:
	//Game* theGame;
	bool m_debug = false;

	IntVec2 dugblock = IntVec2(0, 0);

	//std::map<IntVec2, Chunk*> m_activeChunks
};