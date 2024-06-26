#pragma once
#include "Game/app.hpp"
#include "Game/Player.hpp"
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
//-----------------------------------------------------------------------------------------------


App* g_theApp = nullptr;
Renderer*  g_theRenderer = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
Window* g_theWindow = nullptr;

extern DevConsole* g_theConsole;

bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return false;
}

bool Test_CallbackFunction(EventArgs& args)
{
	int health = args.GetValue("Health", 0);
	int count = args.GetValue("count", 0);
	args.SetValue("count", Stringf("%i", count + 1));
	g_theConsole->AddLine(Rgba8(255, 255, 0), Stringf("Test received; there were %i args; health=%i", args.m_keyValuePairs.size(), health));

	return false;
}

App::App()
{
	InitializeSystems();
}


App::~App()
{

}

void App::InitializeSystems()
{
	EventSystemConfig eventConfig;
	g_theEventSystem = new EventSystem(eventConfig);

	InputConfig inputConfig;
	g_theInputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInputSystem;
	windowConfig.m_windowTitle = "SimpleMiner";
	windowConfig.m_aspectRatio = 2.f;
	g_theWindow = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	Camera camera = Camera();
	camera.SetOrthoView(AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)));

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_camera = camera;
	devConsoleConfig.m_fontFilePathNoExtension = "Data/Fonts/SquirrelFixedFont";
	g_theConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudioSystem = new AudioSystem(audioConfig);
	//g_theRenderer->CreateRenderingContext();

	m_theGame = new Game();
}

void App::Startup()
{

	g_theEventSystem->Startup();
	g_theConsole->Startup();
	g_theInputSystem->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();

	//Camera devConsoleCamera = Camera();

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;

	
	DebugRenderSystemStartup(debugRenderConfig);

	g_theAudioSystem->Startup();
	m_theGame->Startup();

	previousframe = GetCurrentTimeSeconds();

	SubscribeEventCallbackFunction("Quit", Event_Quit);
}

void App::Shutdown()
{
	m_theGame->Shutdown();


	g_theRenderer->Shutdown();
	g_theConsole->Shutdown();
	g_theAudioSystem->Shutdown();
	g_theWindow->Shutdown();
	g_theInputSystem->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theConsole;
	g_theConsole = nullptr;

	delete g_theAudioSystem;
	g_theRenderer;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;

}

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::RunFrame()
{
	//currentframe = GetCurrentTimeSeconds();
	//float deltaSeconds = (float)currentframe - (float)previousframe;
	BeginFrame();	//once this function becomes App::RunFrame()

	//if (m_isPaused == false) // if the program is paused, stop updating the game itself, but continue rendering(check to see if you should just stop everything)
	Update();

	Render();		//once this function becomes App::RunFrame()
	EndFrame();	//once this function becomes App::RunFrame()
	//previousframe = currentframe;
}

void App::BeginFrame()
{
	//g_theCamera.SetOrthoView(Vec2(0, 0), Vec2(200, 100));
	g_theEventSystem->BeginFrame();
	g_theConsole->BeginFrame();
	g_theInputSystem->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	
	DebugRenderBeginFrame();

	g_theAudioSystem->BeginFrame();
	m_theGame->BeginFrame();
	Clock::TickSystemClock();
}

void App::Update()
{
	

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theConsole->ToggleMode(DevConsoleMode::OPEN_FULL);
	}

	if ((g_theWindow->GetHWND() != GetActiveWindow()) || (g_theConsole->GetMode() == DevConsoleMode::OPEN_FULL) || (m_theGame->m_attract == true))
	{
		g_theInputSystem->SetCursorMode(false, false);
	}
	else
	{
		g_theInputSystem->SetCursorMode(true, true);
	}

	if (m_theGame != nullptr)
	{
		if (m_theGame->shuttingDown)
		{
			m_theGame->Shutdown();
			delete m_theGame;
			m_theGame = new Game();
			m_theGame->Startup();
		}
		m_theGame->Update();
	}
}

void App::Render() const
{
	m_theGame->Render();

	g_theConsole->Render(AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)));
}

void App::EndFrame()
{
	for (int i = 0; i < 256; i++)
	{
		g_theInputSystem->m_keyCodesReleased[i] = g_theInputSystem->m_keyCodesDown[i];
	}
	g_theEventSystem->EndFrame();
	g_theConsole->EndFrame();
	g_theInputSystem->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();

	DebugRenderEndFrame();

	g_theAudioSystem->EndFrame();
	m_theGame->EndFrame();
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	//delete m_theGame;
	//m_theGame = nullptr;

	return false;
}




