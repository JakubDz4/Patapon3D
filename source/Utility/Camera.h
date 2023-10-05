#pragma once
#include <SDL.h>
#include "vendor/glm/gtc/matrix_transform.hpp"
#include "Time.h"
#include "numbersGenerator.hpp"

class Camera
{
public:
	glm::vec3 m_pos;
	glm::vec3 m_direction;
	glm::vec3 m_up;
	glm::mat4 m_lookAt;

	static float earthQuake;
	float originalYaw, originalPitch=90.0f;

	float yaw, pitch;
	const float sensitivity = 5.0f;
	const float speed = 10.0f;
public:									//4.5
	Camera(glm::vec3 pos = glm::vec3(0.0f, 2.55f, 15.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float _yaw = -90.0f, float _pitch = -9.55f)
		: m_pos(pos), m_up(up), m_lookAt(glm::mat4(1.0f)), yaw(_yaw), pitch(_pitch)
	{
		calcDir();
		SDL_SetRelativeMouseMode(SDL_TRUE);
	};

	inline void updateView()
	{
		manageEarthQuake();
		m_lookAt = glm::lookAt(m_pos, m_direction + m_pos, m_up);
	}

	glm::mat4& getView()
	{
		return m_lookAt;
	}

	void handleEvent(SDL_Event& ev)
	{

		switch (ev.key.keysym.sym) 
		{
			case SDLK_s:
				m_pos -= glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z)) * speed * Time::deltaTime;
				break;

			case SDLK_w:
				m_pos += glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z)) * speed * Time::deltaTime;
				break;

			case SDLK_d:
				m_pos += glm::cross(glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z)), glm::vec3(0.0f, 1.0f, 0.0f)) * speed * Time::deltaTime;
				break;

			case SDLK_a:
				m_pos -= glm::cross(glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z)), glm::vec3(0.0f, 1.0f, 0.0f)) * speed * Time::deltaTime;
				break;

			case SDLK_SPACE:
				m_pos.y += speed * Time::deltaTime;
				break;

			case SDLK_LCTRL:
				m_pos.y -= speed * Time::deltaTime;
				break;
		}
	}

	void handleMouse(SDL_Event& ev)
	{
		float xoffset = (float)ev.motion.xrel;
		float yoffset = (float)ev.motion.yrel;

		xoffset *= sensitivity * Time::deltaTime;
		yoffset *= sensitivity * Time::deltaTime;

	//	yoffset = 0.0f;

		yaw += xoffset;
		pitch += yoffset;
		if (pitch > 89.0f)
			pitch = 89.0f;
		else if (pitch < -89.0f)
			pitch = -89.0f;

		calcDir();
	}

	inline void move()
	{
		m_pos.x += globalSpeed;
	}

	inline void calcDir()
	{
		float tmp = cos(glm::radians(pitch));
		m_direction.x = cos(glm::radians(yaw)) * tmp;
		m_direction.y = -sin(glm::radians(pitch));
		m_direction.z = sin(glm::radians(yaw)) * tmp;
		m_direction = glm::normalize(m_direction);
	}

	inline void manageEarthQuake()
	{
		if (earthQuake)
		{
			if(originalPitch == 90.0f)
			{
				originalYaw = yaw;
				originalPitch = pitch;
			}

			float randomy = RandomNumbers::getInstance().getDistribution("randomM", -1.0f, 1.0f);
			float randomp = RandomNumbers::getInstance().getDistribution("randomM", -1.0f, 1.0f);

			earthQuake -= Time::deltaTime;
			if (earthQuake < 0.0f)
			{
				yaw = originalYaw;
				pitch = originalPitch;
				calcDir();

				originalPitch = 90.0f;
				earthQuake = 0.0f;

				return;
			}

			yaw = originalYaw + earthQuake * randomy;
			pitch = originalPitch + earthQuake * randomp;

			calcDir();
		}
	}
};

float Camera::earthQuake = 0.0f;