#include "States.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "Engine.h"
#include "EventManager.h"
#include "PlatformPlayer.h"
#include "CollisionManager.h"
#include "SoundManager.h"
#include "Primitives.h"
#include "Button3.h"
#include "Sprites.h"

#include <iostream>
using namespace std;

void State::Render()
{
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}

void State::Resume(){}

GameObject* State::GetGo(const std::string& s)
{ // Using 'std' just to show origin.
	auto it = std::find_if(m_objects.begin(), m_objects.end(), 
		[&](const std::pair<std::string, GameObject*>& element)
		{
			return element.first == s;
		});
	if (it != m_objects.end())
		return it->second;
	else return nullptr;
}

auto State::GetIt(const std::string& s)
{ 
	auto it = std::find_if(m_objects.begin(), m_objects.end(),
		[&](const std::pair<std::string, GameObject*>& element)
		{
			return element.first == s;
		});
	return it;
}

// Begin TitleState
TitleState::TitleState(){}

void TitleState::Enter()
{
	TEMA::Load("Img/Title.png", "title");
	TEMA::Load("Img/button.png", "play");
	TEMA::Load("Img/TitleBack.jpg", "bg");
	SOMA::Load("Aud/Title.mp3", "title", SOUND_MUSIC);
	m_objects.push_back(pair<string, GameObject*>("bg",
		new Image({ 0, 0, 1920, 1200 }, { 0, 0, 1024, 768 }, "bg")));
	m_objects.push_back(pair<string, GameObject*>("title",
		new Image({ 0, 0, 800, 156 }, { 112, 100, 800, 156 }, "title")));
	m_objects.push_back(pair<string, GameObject*>("play",
		new PlayButton({ 0, 0, 400, 100 }, { 412, 384, 200, 50 }, "play")));
	SOMA::AllocateChannels(16);
	SOMA::SetMusicVolume(32);
	SOMA::PlayMusic("title", -1, 2000);
}

void TitleState::Update()
{
	if (EVMA::KeyPressed(SDL_SCANCODE_P))
	{
		STMA::ChangeState(new GameState());
	}
	else if (EVMA::KeyPressed(SDL_SCANCODE_L))
	{
		STMA::PushState(new LoseState());

	}
	for (auto const& i : m_objects)
	{
		i.second->Update();
		if (STMA::StateChanging()) return;
	}
}

void TitleState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	for (auto const& i : m_objects)
		i.second->Render();
	State::Render();
}

void TitleState::Exit()
{
	TEMA::Unload("title");
	TEMA::Unload("play");
	TEMA::Unload("bg");
	SOMA::StopMusic();
	SOMA::Unload("title", SOUND_MUSIC);
	for (auto& i : m_objects)
	{
		delete i.second;
		i.second = nullptr; // ;)
	}
}
// End TitleState

// Begin GameState
GameState::GameState(){}

void GameState::Enter() // Used for initialization.
{
	TEMA::Load("Img/background.png", "bg");
	TEMA::Load("Img/Sprites.png", "sprites");
	SOMA::Load("Aud/Engines.wav", "engines", SOUND_SFX);
	SOMA::Load("Aud/Fire.wav", "fire", SOUND_SFX);
	SOMA::Load("Aud/Explode.wav", "explode", SOUND_SFX);
	SOMA::Load("Aud/Wings.mp3", "wings", SOUND_MUSIC);
	//SOMA::Load("Aud/Danger.mp3", "danger", SOUND_MUSIC);
	m_objects.push_back(pair<string, GameObject*>("bg",
		new Image({ 0, 0, 1024, 768 }, { 0, 0, 1024, 768 }, "bg")));
	m_objects.push_back(pair<string, GameObject*>("astf",
		new AsteroidField(24)));
	m_objects.push_back(pair<string, GameObject*>("ship",
		new ShipAsteroids({ 0, 0, 100, 100 }, { 462.0f, 334.0f, 100.0f, 100.0f })));
	SOMA::SetSoundVolume(16);
	SOMA::SetMusicVolume(32);
	SOMA::PlayMusic("wings", -1, 2000);
}

void GameState::Update()
{
	if (EVMA::KeyPressed(SDL_SCANCODE_X))
	{
		STMA::ChangeState(new TitleState());
	}
	else if (EVMA::KeyPressed(SDL_SCANCODE_P))
	{
		STMA::PushState(new PauseState());
		
	}
	else if (EVMA::KeyPressed(SDL_SCANCODE_L))
	{
		STMA::PushState(new LoseState());
		
	}
	for (auto const& i : m_objects)
	{
		i.second->Update();
		if (STMA::StateChanging()) return;
	}
	// Check collision. Player vs. asteroids.
	if (GetGo("ship") != nullptr)
	{
		vector<Asteroid*>* field = &static_cast<AsteroidField*>(GetGo("astf"))->GetAsteroids();
		ShipAsteroids* ship = static_cast<ShipAsteroids*>(GetGo("ship"));
		for (unsigned int i = 0; i < field->size(); i++)
		{
			Asteroid* ast = field->at(i);
			if (COMA::CircleCircleCheck(ship->GetCenter(), ast->GetCenter(),
				ship->GetRadius(), ast->GetRadius()))
			{
				SOMA::PlaySound("explode");
				delete GetGo("ship");
				m_objects.erase(GetIt("ship")); // Erases whole ship std::pair.
				m_objects.shrink_to_fit();
				return;
			}
		}
		// Collision of bullets and asteroids.
		vector<Bullet*>* bullets = &ship->GetBullets();
		for (unsigned int i = 0; i < bullets->size(); i++)
		{
			Bullet* bul = bullets->at(i);
			for (unsigned int j = 0; j < field->size(); j++ )
			{
				Asteroid* ast = field->at(j);
				if (COMA::CircleCircleCheck(bul->GetCenter(), ast->GetCenter(),
					bul->GetRadius(), ast->GetRadius()))
				{
					SOMA::PlaySound("explode");
					delete bul;
					bullets->erase(bullets->begin() + i);
					bullets->shrink_to_fit();
					delete ast;
					field->erase(field->begin() + j);
					field->shrink_to_fit();
					return;
				}
			}
		}
	}
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	for (auto const& i : m_objects)
		i.second->Render();
	if ( dynamic_cast<GameState*>(STMA::GetStates().back()) ) // Check to see if current state is of type GameState
		State::Render();
}

void GameState::Exit()
{
	TEMA::Unload("bg");
	TEMA::Unload("sprites");
	SOMA::StopSound();
	SOMA::StopMusic();
	SOMA::Unload("engines", SOUND_SFX);
	SOMA::Unload("fire", SOUND_SFX);
	SOMA::Unload("explode", SOUND_SFX);
	SOMA::Unload("wings", SOUND_MUSIC);
	for (auto& i : m_objects)
	{
		delete i.second; // De-allocating GameObject*s
		i.second = nullptr; // ;)
	}
}

void GameState::Resume(){}
PauseState::PauseState()
{}

void PauseState::Enter()
{
	TEMA::Load("Img/Save.png", "exit");
	TEMA::Load("Img/restart.png", "Restart");
	m_objects.push_back(pair<string, GameObject*>("Restart",
		new RestartButton({ 0, 0, 200, 100 }, { 412, 200, 200, 50 }, "Restart")));
	m_objects.push_back(pair<string, GameObject*>("exit",
		new ExitButton({ 0, 0, 200, 100 }, { 412, 384, 200, 50 }, "exit")));
	
}

void PauseState::Update()
{
	if (EVMA::KeyPressed(SDL_SCANCODE_R))
		STMA::PopState();
	if (EVMA::KeyPressed(SDL_SCANCODE_X))
		 STMA::ChangeState(new TitleState());
	 if (EVMA::KeyPressed(SDL_SCANCODE_L))
		STMA::ChangeState(new LoseState());

	
	for (auto const& i : m_objects)
	{
		i.second->Update();
		if (STMA::StateChanging()) return;
		
	}
}

void PauseState::Render()
{
	STMA::GetStates().front()->Render();
	SDL_SetRenderDrawBlendMode(Engine::Instance().GetRenderer(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 255, 255);
	SDL_Rect rect = { 256,128,521,521 };
	SDL_RenderFillRect(Engine::Instance().GetRenderer(), &rect);
	for (auto const& i : m_objects)
		i.second->Render();
	State::Render();
}

void PauseState::Exit()
{
	TEMA::Unload("exit");
	TEMA::Unload("Restart");
	for (auto& i : m_objects)
	{
		delete i.second; // De-allocating GameObject*s
		i.second = nullptr; // ;)
	}
}

LoseState::LoseState()
{}

void LoseState::Enter()
{
	TEMA::Load("Img/End.jpg", "end");
	TEMA::Load("Img/restart.png", "Restart");
	SOMA::Load("Aud/Wings.mp3", "wings", SOUND_MUSIC);
	m_objects.push_back(pair<string, GameObject*>("end",
		new Image({ 0, 0, 1024, 768 }, { 0, 0, 1024, 768 }, "end")));
	m_objects.push_back(pair<string, GameObject*>("Restart",
		new RestartButton({ 0, 0, 200, 100 }, { 412, 500, 200, 50 }, "Restart")));
}

void LoseState::Update()
{
	if (EVMA::KeyPressed(SDL_SCANCODE_N))
		STMA::ChangeState(new GameState());
	if (EVMA::KeyPressed(SDL_SCANCODE_E))
		STMA::ChangeState(new TitleState());

	for (auto const& i : m_objects)
	{
		i.second->Update();
		if (STMA::StateChanging()) return;

	}
}

void LoseState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	for (auto const& i : m_objects)
		i.second->Render();
	
		State::Render();
}

void LoseState::Exit()
{
	TEMA::Unload( "end");
	TEMA::Unload("Restart");
	SOMA::Unload("wings", SOUND_MUSIC);
	for (auto& i : m_objects)
	{
		delete i.second; // De-allocating GameObject*s
		i.second = nullptr; // ;)
	}
}
// End GameState