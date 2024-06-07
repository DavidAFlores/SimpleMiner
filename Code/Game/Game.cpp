
#include "Game.hpp"
#include "Game/Entity.hpp"
#include "Game/app.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"


extern InputSystem* g_theInputSystem;
extern AudioSystem* g_theAudioSystem;
bool m_debug = false;
RandomNumberGenerator* g_theRandom = nullptr;
//Camera m_worldCamera;
Camera m_screenCamera;
extern DevConsole* g_theConsole;
extern Window* g_theWindow;

Texture* g_terrainTexture = nullptr;
SpriteSheet* g_terrainSpriteSheet = nullptr;

Game::Game()
{
	SubscribeEventCallbackFunction("SetGameScale", Event_SetGameTimeScale);
	SubscribeEventCallbackFunction("Controls", Event_Controls);

	//m_worldCamera

	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
}

Game::~Game()
{
	delete m_theWorld;
	m_theWorld = nullptr;
}

void Game::Startup()
{
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);


	m_entities.push_back(new Player(this, Vec3(0, 0, 0)));
	 
	m_player = (Player*)m_entities[0];

	float aspect = g_theWindow->GetAspect();

	m_player->m_playerCam.SetPerspView(aspect, 60.f, 0.1f, 1000.f);
	m_player->m_playerCam.SetRenderBasis(Vec3::DIRECTX11_IBASIS, Vec3::DIRECTX11_JBASIS, Vec3::DIRECTX11_KBASIS);

	g_terrainTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png");
	g_terrainSpriteSheet = new SpriteSheet(*g_terrainTexture, IntVec2(64, 64));

	BlockDef::InitializeBlockDefs();

	m_theWorld = new World(this, m_player);

	m_theWorld->Startup();

	g_theRandom = new RandomNumberGenerator();
	m_gameClock = new Clock();

	

	FireEvent("Controls");
}

void Game::StartGame()
{
	if (!m_attract)
	{
		m_triOffset1 = 800;
		m_triOffset2 = -1000;
		//m_playerShip = new PlayerShip(this, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));

		float scale = 2.f;
		float length = .2f;

		DebugAddWorldArrow(Vec3::ZERO, Vec3::WORLD_XBASIS * scale, .7f, length, -1.f, Rgba8::RED, Rgba8::RED);
		DebugAddWorldArrow(Vec3::ZERO, Vec3::WORLD_YBASIS * scale, .7f, length, -1.f, Rgba8::GREEN, Rgba8::GREEN);
		DebugAddWorldArrow(Vec3::ZERO, Vec3::WORLD_ZBASIS * scale, .7f, length, -1.f, Rgba8::BLUE, Rgba8::BLUE);
	}																 

}

void Game::Shutdown()
{
	delete m_theWorld;
	m_theWorld = nullptr;

	shuttingDown = true;
}

void Game::Update()
{
	float deltaSeconds = m_gameClock->GetDeltaSeoconds();

	if (g_theInputSystem->WasKeyJustPressed('P'))
		Pause();

	if (g_theInputSystem->WasKeyJustPressed('T'))
		SlowMo();

	if (g_theInputSystem->WasKeyJustPressed('O'))
	{
		NextFrame();
		// 		m_isPaused = false;
		// 		m_nextFrame = true;
	}

	if (m_attract)
	{
		if (g_theInputSystem->WasKeyJustPressed(' ') || g_theInputSystem->m_controllers[0]->WasButtonJustPressed(XBOXBUTTON_START) || g_theInputSystem->m_controllers[0]->WasButtonJustPressed(XBOXBUTTON_A) || g_theInputSystem->WasKeyJustPressed('N'))
		{
			m_attract = false;
			StartGame();
		}
		if (g_theInputSystem->WasKeyJustPressed(27) || g_theInputSystem->m_controllers[0]->GetLeftTrigger() == 1.f)
		{
			g_theApp->HandleQuitRequested();
		}

		m_triOffset1 += m_triangleMoveSpeed * deltaSeconds;
		m_triOffset2 += m_triangleMoveSpeed * deltaSeconds;

		if (m_triOffset1 >= 2400.f)
		{
			m_triOffset1 = -800.f;
		}

		if (m_triOffset2 >= 2400.f)
		{
			m_triOffset2 = -800.f;
		}

	}

	if (g_theInputSystem->WasKeyJustPressed(27) || g_theInputSystem->m_controllers[0]->GetLeftTrigger() == 1.f)
	{
		Shutdown();
	}

	if (m_victoryTimer <= 0.f)
	{
		Shutdown();
	}

	if (m_victory == true)
	{
		m_victoryTimer -= deltaSeconds;
	}

	if (m_deathTimer <= 0.f)
	{
		Shutdown();
	}

	if (m_restartGame == true)
	{
		m_deathTimer -= deltaSeconds;
	}

	if (m_isSlowMo) { deltaSeconds = deltaSeconds / 10; }

	if (!shuttingDown)
	{

		if (!m_attract)
		{
			//m_entities

			if (m_isPaused == false)
			{
				m_player->HandleInput(deltaSeconds);

				for (int i = 0; i < m_entities.size(); i++)
				{
					m_entities[i]->Update(deltaSeconds);
				}

				m_theWorld->Update(deltaSeconds);
			}
			if (m_nextFrame)
			{
				m_isPaused = true;
				m_nextFrame = false;
			}
		}
	}

	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));

	float aspect = g_theWindow->GetAspect();

	if (!shuttingDown)
	{
		m_player->m_playerCam.SetPerspView(aspect, 60.f, 0.1f, 1000.f);
	}
}

void Game::BeginFrame()
{
	
}

void Game::EndFrame()
{
	if(m_theWorld)
	m_theWorld->EndFrame();

	std::string string1 = Stringf("Time: %.1f, FPS: %.1f, TimeDilation: %.1f", m_gameClock->GetTotalSeconds(), 1.f / m_gameClock->GetDeltaSeoconds(), m_gameClock->GetTimeScale());

	DebugAddScreenText(string1, Vec2(1400.f, 600.f), 200.f, Vec2(1.f, 1.f), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);

	Mat44 translation;

	std::string string2 = Stringf("X: %.1f, Y: %.1f, Z:%1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z);

	DebugAddMessage(string2, translation, 0.f, Rgba8::RED, Rgba8::RED);
}

void Game::Render()
{
	g_theRenderer->ClearScreen(Rgba8(180, 180, 180, 255));
	g_theRenderer->BeginCamera(m_player->m_playerCam);

	if (!m_attract)
	{
		if (m_theWorld != nullptr)
		{
			m_theWorld->Render();
		}

		for (int i = 0; i < m_entities.size(); i++)
		{
			if (m_entities[i] != nullptr)
			{
				m_entities[i]->Render();
			}
		}

		DebugRenderWorld(m_player->m_playerCam);
	}

	g_theRenderer->EndCamera(m_player->m_playerCam);

#pragma region ScreenCamera

	//create new camera for UI stuff that is bigger than the world camera
	g_theRenderer->BeginCamera(m_screenCamera);


	//render attract screen on UI
	if (m_attract)
	{
		RenderAttractScreen();
		//return;
	}

	//render game if we are not in attract mode
	if (!m_attract)
	{
		DebugRenderScreen(m_screenCamera);
// 		Vertex_PCU tempPlay1Verts[3];
// 
// 		Vertex_PCU m_playButtonVerts[3];
// 
// 		m_playButtonVerts[0] = Vertex_PCU(-1.5f, 1.5f, 102, 153, 204, 255);
// 		m_playButtonVerts[1] = Vertex_PCU(-1.5f, -1.5f, 102, 153, 204, 255);
// 		m_playButtonVerts[2] = Vertex_PCU(1.5, 0.f, 102, 153, 204, 255);
// 
// 		for (int vertIndex = 0; vertIndex < 3; vertIndex++)
// 		{
// 			tempPlay1Verts[vertIndex] = m_playButtonVerts[vertIndex];
// 		}
// 		TransformVertexArrayXY3D(3, tempPlay1Verts, 80.f, m_rotationoffset, Vec2(m_triOffset1, 400.f));
// 		g_theRenderer->DrawVertexArray(3, tempPlay1Verts);
// 
		if (m_debug == true)
		{
			DebugRender();
		}
	}

	

	g_theRenderer->EndCamera(m_screenCamera);

#pragma endregion ScreenCameraStuff
}

void Game::RenderAttractScreen()
{
	DebugDrawRing(Vec2(800, 400), (sinf(m_gameClock->GetTotalSeconds())  * 100.f) + 100.f, 20, Rgba8(255, 0, 0));
}

void Game::DebugRender()
{
	//DebugDrawRing(m_playerShip->m_position, PLAYER_SHIP_COSMETIC_RADIUS, .1f, Rgba8(255, 0, 255));
	//DebugDrawLine(m_playerShip->m_position, Vec2(PLAYER_SHIP_COSMETIC_RADIUS*CosDegrees(m_playerShip->m_orientationDegrees)+ m_playerShip->m_position.x, PLAYER_SHIP_COSMETIC_RADIUS*SinDegrees(m_playerShip->m_orientationDegrees)+ m_playerShip->m_position.y), .5f, Rgba8(0, 255, 0));
	//DebugDrawLine(m_playerShip->m_position, Vec2(m_playerShip->m_velocity.x + m_playerShip->m_position.x, m_playerShip->m_velocity.y + m_playerShip->m_position.y), .5f, Rgba8(255, 255, 0));
}

void Game::ToggleDebug()
{
	m_debug = !m_debug;
}
	    
void Game::Pause()
{
	m_gameClock->TogglePause();
}

void Game::SlowMo()
{
	if (m_gameClock->GetTimeScale() == .1f)
	{
		m_gameClock->SetTimeScale(1.f);
	}
	else
	{
		m_gameClock->SetTimeScale(.1f);
	}
	//m_isSlowMo = !m_isSlowMo;
}

void Game::NextFrame()
{
	m_gameClock->StepSingleFrame();
}

bool Game::IsAlive(Entity* entity) const
{
	entity;
	return false;
}

bool Event_SetGameTimeScale(EventArgs& args)
{
	float scale = args.GetValue("scale", 1.f);
	g_theApp->m_theGame->SetGameTimeScale(scale);

	return true;
}

bool Event_Controls(EventArgs& args)
{
	UNUSED(args);
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "Controls:");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "E to move Forward");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "S to rotate counter clockwise");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "F to rotate clockwise");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "Spacebar to shoot");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "F1 to turn on DebguDraw");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "T to Slow Down Time");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "O to Step A Single Frame");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "P to Pause");
	g_theConsole->AddLine(Rgba8(200, 200, 0, 200), "Esc to Quit the game/Close the application");

	return true;
}

void Game::SetGameTimeScale(float scale)
{
	m_gameClock->SetTimeScale(scale);
}	
