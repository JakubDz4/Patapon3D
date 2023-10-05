#pragma once
#include "../vendor/glm/ext/vector_float3.hpp"
#include "EntityObiect.hpp"

enum class ColliderType
{
	FRIENDLY,
	NEUTRAL,
	ENEMY,
};


class Collider
{
private:
	const glm::vec3 offsetPoint;

public:
	bool active = true;
	float radius;
	
	glm::vec3 point;
	EntityAlignment aligment;

//	bool isColliding = false;
	Entity* parent;
	Collider* collidedWith;

	Collider(const float _r, const glm::vec3 _p, Entity* entity) :radius(_r), offsetPoint(_p), point(_p), aligment(entity->getAlingment()), parent(entity), collidedWith(nullptr)
	{

	}

	void setPositionOffset(const glm::vec3& pos)
	{
		point = offsetPoint + pos;
	}

	void setPosition(const glm::vec3& pos)
	{
		point = pos;
	}
};