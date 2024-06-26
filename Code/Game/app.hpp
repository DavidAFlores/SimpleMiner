#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
//-----------------------------------------------------------------------------------------------

class App
{
	public:
	App();
	~App();

	void InitializeSystems();

	void Startup();
	void Shutdown();
	void RunFrame();
	void Run();


	bool IsQuitting() const{return m_isQuitting;}

	bool HandleQuitRequested();
	
	static bool Event_Quit(EventArgs& args);

public:
	Game* m_theGame;

private:
	void BeginFrame();
	void Update ();
	void Render() const;
	void EndFrame();
	
private:
	bool m_isQuitting = false;
	double previousframe;
	double currentframe;
	//Vec2 m_shipPos;
};