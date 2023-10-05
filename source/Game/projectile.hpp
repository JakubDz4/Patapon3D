#pragma once
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../Model/model.h"
#include "../Utility/Time.h"
#include "../Utility/numbersGenerator.hpp"
#include "EntityObiect.hpp"
#include "Collider.hpp"
#include <random>
#include "../Music/BattleAudioMaster.hpp"
#include "hitExplosion.hpp"
#include "../Utility/ParticleSystem.hpp"
#include "../debug/debugShow.hpp"

static const float gravity = 1.4f;//2.4

enum class ProjectileBehavior
{
	STAY,
	DONTSHOW,
	FADE,
};

class Projectile : public Entity
{
private:
	glm::vec3 dir;
	float projLen;

	glm::vec3 projStart, projEnd;

	float velocity;// = 0.1;// 0.8f;

	glm::vec3 force;
	float lastAngle;
	float curveMod;

	Collider* collider;
	
	int damage;
	float knockback;

	ProjectileBehavior behave;
	ProjectileSound projSound;

	ParticleEmitter* particles;

	const float maxVisible=0.8f;

...
};

struct ProjectileVertex
{
	glm::mat4 model;
	float visible;
};

class ProjectilePool
{
private:
	static ProjectilePool* projectilePool;
	std::vector<Projectile> projectiles;
	std::unordered_map<Model*, std::vector<ProjectileVertex>> projMap;

	Texture diffuseTex; //for projectile atlas textures for performance
	Texture specularTex;

	unsigned int pointer = 0;
	const unsigned int maxSize = 128;
	bool full = false;

	unsigned int Vbo;

public:
	ProjectilePool(): Vbo(0), diffuseTex("resources/models/Items/Throwable/textures/projectiles.png"), specularTex("resources/models/Items/Throwable/textures/projectilesSpec.jpg")
	{
		projectiles.reserve(maxSize);

		auto maxVertexSize = sizeof(ProjectileVertex) * maxSize;

		glGenBuffers(1, &Vbo);
		glBindBuffer(GL_ARRAY_BUFFER, Vbo);
		glBufferData(GL_ARRAY_BUFFER, maxVertexSize, nullptr, GL_STREAM_DRAW);

		for (auto& proj : DataProjectilesArrows)
		{
			glBindVertexArray(proj.GetMeshes()[0].VAO);
			setLayoutAttributes();
		}
		for (auto& proj : DataProjectilesSpears)
		{
			glBindVertexArray(proj.GetMeshes()[0].VAO);
			setLayoutAttributes();
		}
		for (auto& proj : DataProjectilesOthers)
		{
			glBindVertexArray(proj.getVisualModel()->GetMeshes()[0].VAO);
			setLayoutAttributes();
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	~ProjectilePool()
	{
		glDeleteBuffers(1, &Vbo);
	}

	ProjectilePool(ProjectilePool& other) = delete;

	void operator=(const ProjectilePool&) = delete;

	static ProjectilePool* GetInstance();

	void createProjectileWild(RangeItem* item, glm::vec3 startPos, float velocity, glm::vec3 initDirection, EntityAlignment alingment, float _curveMod = 1.0f, float scale = 0.2f, 
		ProjectileBehavior behave = ProjectileBehavior::STAY, ProjectileSound sound = ProjectileSound::ARROW, ParticleEmitter* particles = nullptr)
	{
		Debug::Timer a("ProjectilePool-createProjectileWild");

		if (full)
		{
			projectiles[pointer++].create(item, velocity, startPos, _curveMod, initDirection, scale, alingment, behave, sound, particles);
			if (pointer == maxSize)
				pointer = 0;
		}
		else
		{
			projectiles.emplace_back(item, velocity, startPos, _curveMod, initDirection, scale, alingment, behave, sound, particles);
			if (projectiles.size() == maxSize)
				full = true;
		}
	}

	void createProjectile(RangeItem* item, glm::vec3 startPos, glm::vec3 initDirection, float _curveMod = 1.0f)
	{
		float matchedVelocity = item->velocity;
		switch (stageBar)
		{
		case MusicQualityBar::epic:
			randomizeFever(initDirection, _curveMod, matchedVelocity);
			break;
		case MusicQualityBar::good:
			randomizeGood(initDirection, _curveMod, matchedVelocity);
			break;
		default:
			randomizeWeak(initDirection, _curveMod, matchedVelocity);
			break;
		}

		if (full)
		{
			projectiles[pointer++].create(item, matchedVelocity, startPos, _curveMod, initDirection);
			if (pointer == maxSize)
				pointer = 0;
		}
		else
		{
			projectiles.emplace_back(item, matchedVelocity, startPos, _curveMod, initDirection);
			if (projectiles.size() == maxSize)
				full = true;
		}
	}

	void createProjectile(RangeItem* item, glm::vec3 startPos, glm::vec3 targetPos, glm::vec3 initDirection, float maxRange, float _curveMod = 1.0f)
	{
		Debug::Timer a("ProjectilePool-createProjectile");

		float matchedVelocity = getmatchedVelocity(initDirection, startPos, targetPos, item->velocity);

		switch (stageBar)
		{
		case MusicQualityBar::epic:
			randomizeFever(initDirection, _curveMod, matchedVelocity);
			break;
		case MusicQualityBar::good:
			randomizeGood(initDirection, _curveMod, matchedVelocity);
			break;
		default:
			randomizeWeak(initDirection, _curveMod, matchedVelocity);
			break;
		}
		
		if (full)
		{
			projectiles[pointer++].create(item, matchedVelocity,  startPos, _curveMod, initDirection);
			if (pointer == maxSize)
				pointer = 0;
		}
		else
		{
			projectiles.emplace_back(item, matchedVelocity, startPos, _curveMod, initDirection);
			if (projectiles.size() == maxSize)
				full = true;
		}
	}

	std::vector<Projectile>& getProjectiles()
	{
		return projectiles;
	}

	inline void onActions()
	{
		//if less projectiles, then better (lower than ~40)
		/*{
			Debug::Timer a("ProjectilePool-onActions");//6.2k

			projMap.clear();

			for (auto& proj : ProjectilePool::GetInstance()->getProjectiles())
			{
				if (proj.active)
				{
					proj.move();

					Model* vModel = proj.getVisualModel();
					glm::mat4& matModel = proj.getMatrixModel();

					projMap[vModel].emplace_back(matModel, proj.getVisibility());
				}
				else if (proj.visible >= 0.0f)
				{
					Model* vModel = proj.getVisualModel();
					glm::mat4& matModel = proj.getMatrixModel();

					projMap[vModel].emplace_back(matModel, proj.getVisibility());

					proj.manageBehaviorWhenUnActive();
				}
			}
		}*/

		Debug::Timer a("ProjectilePool-onActions");

		projMap.clear();
		projMap.reserve(DataProjectilesSpears.size() + DataProjectilesArrows.size() + DataProjectilesOthers.size());

		for (auto& proj : ProjectilePool::GetInstance()->getProjectiles())
		{
			if (proj.active)
			{
				proj.move();

				Model* vModel = proj.getVisualModel();
				glm::mat4& matModel = proj.getMatrixModel();

				auto& vertexes = projMap[vModel];
				if (vertexes.empty())
					vertexes.reserve(maxSize);

				vertexes.emplace_back(matModel, proj.getVisibility());
			}
			else if (proj.visible >= 0.0f)
			{
				Model* vModel = proj.getVisualModel();
				glm::mat4& matModel = proj.getMatrixModel();

				auto& vertexes = projMap[vModel];
				if (vertexes.empty())
					vertexes.reserve(maxSize);

				vertexes.emplace_back(matModel, proj.getVisibility());

				proj.manageBehaviorWhenUnActive();
			}
		}
	}
	
	void instancedDraw(Shader& shader)
	{
			Debug::Timer a("ProjectilePool-instancedDraw");//2k - 2200% boost by just using texture atlas

			glBindBuffer(GL_ARRAY_BUFFER, Vbo);
			shader.SetUniform1i("texture_diffuse1", 0);
			shader.SetUniform1i("texture_specular1", 1);
			diffuseTex.Bind();
			specularTex.Bind(1);
			/*TODO:
			Zastanowic sie, czy polaczyc onActions i Draw*/
			for (auto& proj : projMap)
			{
				auto& mesh = proj.first->GetMeshes()[0];
				auto& projVertex = proj.second;

				glBindVertexArray(mesh.VAO);

				auto count = projVertex.size();

				glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(ProjectileVertex), projVertex.data());

				//no shader arg, so draw without binding tex
				mesh.DrawInstanced(count);
			}

		/* {
			Debug::Timer a("ProjectilePool-instancedDrawNoTex");//44.5k

			glBindBuffer(GL_ARRAY_BUFFER, Vbo);
			for (auto& proj : projMap)
			{
				auto& mesh = proj.first->GetMeshes()[0];
				auto& projVertex = proj.second;

				glBindVertexArray(mesh.VAO);

				auto count = projVertex.size();

				glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(ProjectileVertex), projVertex.data());


				//no shader arg, so draw without binding tex
				mesh.DrawInstanced(shader, count);
				//mesh.DrawInstanced(shader, count);
			}
		}*/

	}

	private: 

	void setLayoutAttributes()
	{
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 17, (void*)(sizeof(float) * 0));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 17, (void*)(sizeof(float) * 4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 17, (void*)(sizeof(float) * 8));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 17, (void*)(sizeof(float) * 12));
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 17, (void*)(sizeof(float) * 16));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
	}

	float getmatchedVelocity(glm::vec3& dir, glm::vec3& startPos, glm::vec3& targetPos, float oldVel)
	{
		float range = glm::abs(targetPos.x - startPos.x);

		auto len = glm::length(glm::vec2(dir.x, dir.y));
		auto sina = dir.y / len;
		auto cosa = dir.x / len;

		/*if (oldVel < 1.45f)
		{
			range *= (oldVel / 1, 45);
		}*/

		return sqrtf(range * gravity / (10 * 2 * sina * cosa));
	}

	void randomizeWeak(glm::vec3& dir, float& _curveMod, float& velocity)
	{
		auto& randomGen = RandomNumbers::getInstance();

		auto velGap = randomGen.getDistribution("mediumVelGap", 0.9f, 1.1f);
		auto yGap = randomGen.getDistribution("mediumYGap", 0.9f, 1.4f);
		auto curveGap = randomGen.getDistribution("mediumCurvGap", 0.8f, 2.1f);
		auto dirZ = randomGen.getDistribution("mediumDirGap", -0.06f, 0.06f);

		_curveMod *= curveGap;
		velocity *= velGap;
		dir.y *= yGap;
		dir.z = dirZ;
		dir = glm::normalize(dir);
	}

	void randomizeGood(glm::vec3& dir, float& _curveMod, float& velocity)
	{
		auto& randomGen = RandomNumbers::getInstance();

		auto velGap = randomGen.getDistribution("mediumVelGap", 1.0f, 1.1f);
		auto yGap = randomGen.getDistribution("mediumYGap", 1.0f, 1.3f);
		auto curveGap = randomGen.getDistribution("mediumCurvGap", 0.9f, 2.0f);
		auto dirZ = randomGen.getDistribution("mediumDirGap", -0.05f, 0.05f);

		_curveMod *= curveGap;
		velocity *= velGap;
		dir.y *= yGap;
		dir.z = dirZ;
		dir = glm::normalize(dir);
	}

	/*//void randomizeFever(glm::vec3& dir, float& _curveMod, float& velocity)
	//{
	//	auto& randomGen = RandomNumbers::getInstance();
	//	auto genSmallx = randomGen.getDistribution("smallGap", 0.7f, 1.3f);
	//	auto genSmally = randomGen.getDistribution("smallGap", 0.7f, 1.3f);
	//	auto dirz = randomGen.getDistribution("DirGap", -0.05f, 0.05f);

	//	auto velGap = randomGen.getDistribution("smallVelGap", 0.98f, 1.02f);
	//	auto yGap = randomGen.getDistribution("smallYGap", 1.1f, 1.25f);
	//	auto curveGap = randomGen.getDistribution("smallCurvGap", 0.9f, 1.3f);
	//	auto dirZ = randomGen.getDistribution("smallDirGap", -0.04f, 0.04f);

	//	//auto gen(gen);
	//	_curveMod *= curveGap;
	//	//	dir.x *= xGap;
	//	velocity *= velGap;
	//	dir.y *= yGap;
	//	dir.z = dirZ;
	//	dir = glm::normalize(dir);
	//}*/

	void randomizeFever(glm::vec3& dir, float& _curveMod, float& velocity)
	{
		auto& randomGen = RandomNumbers::getInstance();

		auto velGap = randomGen.getDistribution("smallVelGap", 1.0f, 1.04f);
		auto yGap = randomGen.getDistribution("smallYGap", 1.1f, 1.25f);
		auto curveGap = randomGen.getDistribution("smallCurvGap", 1.0f, 1.6f);
		auto dirZ = randomGen.getDistribution("smallDirGap", -0.04f, 0.04f);

		_curveMod *= curveGap;
		velocity *= velGap;
		dir.y *= yGap;
		dir.z = dirZ;
		dir = glm::normalize(dir);
	}
};

ProjectilePool* ProjectilePool::projectilePool = nullptr;

ProjectilePool *ProjectilePool::GetInstance()
{
	if (projectilePool == nullptr) {
		projectilePool = new ProjectilePool();
	}
	return projectilePool;
}

//ProjectilePool::projectilePool = nullptr;


//static std::vector<Projectile> projectiles;