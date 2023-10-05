#pragma once
#include "../Model/model.h"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../Database/EntitiesVisualData.hpp"

enum class EntityAlignment {
    FRIENDLY,
    NEUTRAL,
    ENEMY
};

class Entity
{
protected:
	Model* visualModel;
	glm::mat4 modelMatrix;
    EntityAlignment alingment;

public:
    Entity(EntityClass EntityType, glm::vec3 position = glm::vec3(0.0f), EntityAlignment _alingment = EntityAlignment::NEUTRAL)
        : visualModel(&modelData[static_cast<unsigned int>(EntityType)]), modelMatrix(glm::translate(glm::mat4(1.0f), position)), alingment(_alingment)
    {

    }

    Entity(Model* model, glm::mat4 modMat = glm::mat4(1.0f), EntityAlignment _alingment = EntityAlignment::NEUTRAL)
        : visualModel(model), modelMatrix(modMat), alingment(_alingment)
    {

    }

    const inline glm::vec3& getPosition()
    {
        return glm::vec3(modelMatrix[3]);
    }

    const inline glm::mat4& getModelMatrix()
    {
        return modelMatrix;
    }

    inline EntityAlignment getAlingment()
    {
        return alingment;
    }

    virtual int getDamage() 
    {
        return 0;
    }

    virtual float getKnockback()
    {
        return 0;
    }

    virtual void onCollision()
    {

    }

    virtual void onTriggerCollision()
    {

    }
};