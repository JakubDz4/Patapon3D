#pragma once
#include <SDL.h>

class Time
{
private:
	Uint32 prevTime;           
public:
	Time()
	{
		prevTime = SDL_GetTicks();
	}

	void update()
	{
		currentTime = SDL_GetTicks();
		deltaTime = (currentTime - prevTime) / 1000.0f;
		prevTime = currentTime;

	}

	static float deltaTime;
	static float smoothPass;
	static Uint32 currentTime;
};

float Time::deltaTime = 0.0f;
float Time::smoothPass = -2.0f;
Uint32 Time::currentTime = 0;

static const float globalSpeedMax = 1.8f;
static float globalSpeed = globalSpeedMax;