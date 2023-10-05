#pragma once
#include "../Model/model.h"

class Item
{
protected:
	Model visualModel;
	glm::mat4 characterMatrix;
	unsigned int characterMatrixId;

public:
	Item(const std::string& filePath) :visualModel(filePath), characterMatrix(glm::mat4(1.0f)), characterMatrixId(0)
	{

	}

	inline void setCharacterMatrix(glm::mat4& charMat)
	{
		characterMatrix = charMat;
	}

	glm::mat4& getCharacterMatrix()
	{
		return characterMatrix;
	}

	inline void setCharacterMatrixId(unsigned int charMatId)
	{
		characterMatrixId = charMatId;
	}

	unsigned int getCharacterMatrixId()
	{
		return characterMatrixId;
	}

	inline void draw(Shader& shader)
	{
		visualModel.Draw(shader);
	}
};

class RangeItem :public Item
{
public:
	const int damage;
	const float attackSpeed;
	const float velocity;// = 0.1;// 0.8f;
	const float knockback;
	float itemLength;
	float radius;


	Model* projectileVisual;
public:
	RangeItem(const std::string& filePath, Model& _projectileModel, const int _damage, const float _attackSpeed, const float _velocity, const float _knock, const float _radius = 0.05f)
		:Item(filePath), damage(_damage), attackSpeed(_attackSpeed), velocity(_velocity), projectileVisual(&_projectileModel), radius(_radius), knockback(_knock)
	{
		auto bones = projectileVisual->GetBoneInfoMap();
		itemLength = glm::length(bones["bWeaponStart"].offset[3] - bones["bWeaponEnd"].offset[3]) / 5;
	}

	RangeItem(const std::string& filePath, const int _damage, const float _attackSpeed, const float _velocity, const float _knock, const float _radius = 0.05f)
		:Item(filePath), damage(_damage), attackSpeed(_attackSpeed), velocity(_velocity), projectileVisual(nullptr), radius(_radius), knockback(_knock)
	{
		auto bones = visualModel.GetBoneInfoMap();
		itemLength = glm::length(bones["bWeaponStart"].offset[3] - bones["bWeaponEnd"].offset[3]) / 5;
	}

	void setSelfProjectileModel()
	{
		projectileVisual = &visualModel;
	}

	Model* getVisualModel()
	{
		return &visualModel;
	}
};


class MeleeItem :public Item
{
public:
	const int damage;
	const float attackSpeed;
	float range;
public:
	MeleeItem(const std::string& filePath, const int _damage, const float _attackSpeed)
		:Item(filePath), damage(_damage), attackSpeed(_attackSpeed)
	{
		auto bones = visualModel.GetBoneInfoMap();
		range = glm::length(bones["bWeaponStart"].offset[3] - bones["bWeaponEnd"].offset[3]) / 5;
	}
};

class DefenceItem :public Item
{
public:
	const int defence;
	
public:
	DefenceItem(const std::string& filePath, const int _defence)
		:Item(filePath), defence(_defence)
	{
		
	}
};

class VisualItem
{
protected:
	Model *visualModel;
	glm::mat4 characterMatrix;
	unsigned int characterMatrixId;

public:
	VisualItem(Model* model) :visualModel(model), characterMatrix(glm::mat4(1.0f)), characterMatrixId(0)
	{

	}

	inline void setCharacterMatrix(glm::mat4& charMat)
	{
		characterMatrix = charMat;
	}

	glm::mat4& getCharacterMatrix()
	{
		return characterMatrix;
	}

	inline void setCharacterMatrixId(unsigned int charMatId)
	{
		characterMatrixId = charMatId;
	}

	unsigned int getCharacterMatrixId()
	{
		return characterMatrixId;
	}

	inline void draw(Shader& shader)
	{
		visualModel->Draw(shader);
	}
};