#pragma once
#include "../Model/model.h"
#include "../Model/animator.h"
#include "../Database/EntitiesVisualData.hpp"
#include "../Game/EntityObiect.hpp"
#include "../Utility/Time.h"
#include "../Game/Collider.hpp"

struct ColliderSegment
{
	Bone* bone;
	Collider* collider;

	ColliderSegment(Bone* _bone, Collider* _collider) : bone(_bone), collider(_collider)
	{

	}
};

enum class BossBehavior
{
	ANOTHER = -1,

	SLEEP = 0,
	WAIT,

	MOVE,
	MOVEBACK,
	RUN,
	RUNBACK,

	//ESCAPE,

	ATTACK_MID1,
	ATTACK_CLOSE1,

	EnumSize,
};

struct BehaveComponent
{
	float chances = 0.0f;
	BossBehavior behave = BossBehavior::ANOTHER;;
	float combo = 0.0f;

	void reset()
	{
		chances = 0.0f;
		behave = BossBehavior::ANOTHER;
		combo = 0.0f;
	}

	void set(float _ch, BossBehavior _b)
	{
		chances = _ch + combo;
		behave = _b;
	}
};

class BossAi
{
	BossBehavior drawnBehavior;
	Patapon* target;
	//Boss* boss;

	/*distance from origin to closest left point*/
	const float xWidth = 4.33f;//closest left collider point + radius
	const float maxCloseLimit = xWidth / 3.0f;

	float distance = 0.0f;//distance from closest boss point to target
	float distanceRel = 0.0f;

	glm::vec4* bossPos;

	const float conditionMaxBonus = 0.60f;
	const float conditionMinBonus = 0.3f;
	const float randomVal = 1.0f - conditionMaxBonus;
	const float maxBossDistance = 20.0f / 1.6f; //20 = 2*scaleX z layerManager, 1.8f - max dystans troche wiecej niz polowa dlugosci planszy widocznej
	const float comboIncrement = -0.1f;
	const float comboDecrement = 0.03f;

	BehaveComponent behavior[static_cast<int>(BossBehavior::EnumSize)];

public:
	BossBehavior getBehavior(Patapon* _target)
	{
		target = _target;
		float distance = bossPos->x - target->getPosition().x;
		float gameDistance = bossPos->x - target->getGamePos().x;
		float max = 0.0f;
		BehaveComponent* chosenBehavior = nullptr;

		for (int i = 0; i < static_cast<int>(BossBehavior::EnumSize); i++)
		{
			auto b = static_cast<BossBehavior>(i);
			behavior[i].set(returnComponentChance(b, distance, gameDistance), b);
			if (behavior[i].chances > max)
			{
				max = behavior[i].chances;
				chosenBehavior = &behavior[i];
			}
			else if (behavior[i].combo <0.0f)
			{
				behavior[i].combo += comboDecrement;
			}
		}

		chosenBehavior->combo += comboIncrement;

		return chosenBehavior->behave;
	}

	void setPos(glm::mat4& modelMat)
	{
		bossPos = &modelMat[3];
	}

private:

	inline float returnComponentChance(BossBehavior behave, float distance, float gameDistance)
	{
		float randomChances = RandomNumbers::getInstance().getDistribution("bossRand", 0.0f, randomVal);

		switch (behave)
		{
		case BossBehavior::SLEEP:
		{
			return 0.0f;
		}

		case BossBehavior::WAIT:
		{
			return randomChances + conditionMinBonus;
		}

		case BossBehavior::MOVE:
		case BossBehavior::RUN:
		{
			//if (target->getGamePos().x > bossPos->x - xWidth)
			if (xWidth > gameDistance)
				return 0.0f;

			return randomChances + betweenConditionMinMax(distance / maxBossDistance);
		}

		case BossBehavior::MOVEBACK:
		{
			return randomChances + betweenConditionMinMax(1.0f - distance / maxBossDistance);
		}

		case BossBehavior::RUNBACK:
		{
			//if (target->getGamePos().x > bossPos->x - xWidth)
			if (maxCloseLimit > gameDistance)
				return 1.0f;

			return randomChances + betweenConditionMinMax(1.25f - distance / (maxBossDistance*0.5f));
		}

		case BossBehavior::ATTACK_MID1:
		{
			return randomChances + betweenConditionMinMax(1.1f - distance / maxBossDistance);
		}

		case BossBehavior::ATTACK_CLOSE1:
		{
			if (target->getGamePos().x > bossPos->x - xWidth)
				return 1.0f;

			return randomChances + betweenConditionMinMax(1.35f - distance / (maxBossDistance * 0.5f));
		}

		default: assert(0); break;
		}
	}

	inline float betweenConditionMinMax(float val)
	{
		return max(min(val, conditionMaxBonus), conditionMinBonus);
	}
};

class Boss :public Entity
{
	Animator animator;
	AnimationSet* curAnimSet;
	BossBehavior behavior = BossBehavior::SLEEP;
	BossBehavior lastBehavior = BossBehavior::ANOTHER;
	BossBehavior queueBehavior = BossBehavior::ANOTHER;

	BossAi Ai;

	std::vector<ColliderSegment> colliders;
	int HP;

	bool active = true;

	float scale;
	const float colAnimMod;

	const float flameAttackPauseMax = 0.05f;
	float flameAttackPause = 0.0f;

	float lastTime = 0.0f;
	bool animationEnded = false;

	const float movementSpeed = 0.5f * scale;
	bool stepped = false;

	Collider* hitCollider;
	bool attacked = false;

public:
	Boss(EntityClass EntityType, glm::vec3 position = glm::vec3(0.0f), int _HP = 100, float _scale = 7.0f, EntityAlignment _alingment = EntityAlignment::ENEMY)
		:Entity(EntityType, position, _alingment), HP(_HP), colAnimMod(0.1020489f * _scale), scale(_scale)
	{
		curAnimSet = &animationData[static_cast<int>(EntityType)];
		animator.PlayAnimation(&curAnimSet->animations[0]);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(_scale));

		onChangedAnimation();
		Ai.setPos(modelMatrix);
	}

	void updateAnimation()
	{
		if (lastBehavior != behavior)
		{
			lastBehavior = behavior;
			animator.PlayAnimation(&curAnimSet->animations[static_cast<int>(behavior)]);
			onChangedAnimation();
		}
		animator.UpdateAnimation(Time::deltaTime);

		if (lastTime > animator.getCurrentTime())
			animationEnded = true;	

		lastTime = animator.getCurrentTime();
	}

	void draw(Shader& shader)
	{
		updateAnimation();
		auto transforms = animator.GetFinalBoneMatrices();
		shader.SetUniformMat4fv("finalBonesMatrices", transforms, transforms.size());
		shader.SetUniformMat4f("model", modelMatrix);

		/*auto& bones = animator.getCurrentAnimation()->GetBoneIDMap();
		auto offset = bones["bGlowa"].offset;
		offset *= bones["bSzyjaGorna"].offset;
		offset *= bones["bSzyjaDolna"].offset;
		offset *= bones["bTulow"].offset;

		auto mat = glm::translate(glm::mat4(1.0f), glm::vec3(-3.2f, 3.3f, 0.0f));
		auto t = transforms[3] * mat;

		colliders[2]->point = t[3];

		//colliders[2]->point += glm::vec3(3.2f, 3.3f, 0.0f);*/

		/*auto mat = glm::translate(glm::mat4(1.0f), glm::vec3(-1.6f, 3.4f, 0.0f));
		auto t = mat * transforms[39];

		auto c = animator.getCurrentAnimation()->FindBone("bCol1")->GetLocalTransform();

		auto& x = animator.getCurrentAnimation()->GetActiveBones();

		colliders[2]->point.x = c[3][0] * colAnimMod + modelMatrix[3][0];
		colliders[2]->point.y = c[3][2] * colAnimMod + modelMatrix[3][1];*/

		for (auto& col : colliders)
		{
			auto c = col.bone->GetLocalTransform();
			col.collider->point.x = c[3][0] * colAnimMod + modelMatrix[3][0];
			col.collider->point.y = c[3][2] * colAnimMod + modelMatrix[3][1];
		}

		visualModel->Draw(shader);
	}

	void doActions(Patapon* patapon, bool readyAttack)
	{
		if (patapon == nullptr || !active)
			return;

		if(animationEnded)
		{
			behavior = Ai.getBehavior(patapon);
			if (behavior >= BossBehavior::ATTACK_MID1 && !readyAttack)
			{
				queueBehavior = behavior;
				behavior = BossBehavior::WAIT;
			}
			animationEnded = false;
			attacked = false;
		}

		if (readyAttack && queueBehavior != BossBehavior::ANOTHER)
		{
			behavior = queueBehavior;
			queueBehavior = BossBehavior::ANOTHER;
		}

		//DEBUG
		//behavior = BossBehavior::WAIT;

		manageBehavior();
		manageLegKnockback(patapon);
	}

	void manageBehavior()
	{
		switch(behavior)
		{
		case BossBehavior::SLEEP:
				break;

		case BossBehavior::WAIT:
			break;

		case BossBehavior::ATTACK_MID1:
		{
			if (colliders[0].bone->getScale(animator.getCurrentTime()).y > 1.0f)
			{
				BattleAudioMaster::GetInstance()->playDodongaBreath();

				if (flameAttackPause < 0.0f)
				{
					float curve[3] = {		RandomNumbers::getInstance().getDistribution("DodongaFireBreathClose", -4.0f, -2.0),
											RandomNumbers::getInstance().getDistribution("DodongaFireBreathMid", -2.0f, -1.0),
											RandomNumbers::getInstance().getDistribution("DodongaFireBreathLong", -1.0f, -0.4) };

					auto dirZ = RandomNumbers::getInstance().getDistribution("DodongaFireBreathZ", -0.02f, 0.02);

					for (int i = 0; i < 3; i++)
					{
						/*ParticleEmitter* particles = ParticleSystem::GetInstance()->createEmitter(colliders[1].collider->point, glm::normalize(-glm::vec3(0.4f, 1.4f, dirZ)), 0.2f, 1.0f, 40.0f, 0.0f, -1.0f, 1.0f,
						-0.2, 0.0f,0.0f, "dodongaBreath", 45.0f, 0.5f);*/
						ParticleEmitter* particles = ParticleSystem::GetInstance()->createEmitter(colliders[1].collider->point, glm::normalize(glm::vec3(-1.2f, 1.0f, dirZ)), 2.5f, 1.0f, 6.5f, 0.0f, -1.0f, 2.5f,
						-0.8, 0.0f, 0.0f, 0, 126, "dodongaBreath", 60.0f, 0.5f);
						ProjectilePool::GetInstance()->createProjectileWild(&DataProjectilesOthers[static_cast<int>(ProjectileType::FIREBALL)], colliders[1].collider->point, 0.5f, glm::normalize(glm::vec3(1.0f, 0.1f, dirZ)),
							EntityAlignment::ENEMY, curve[i], 0.25f, ProjectileBehavior::DONTSHOW, ProjectileSound::FLAME, particles);
					}

					flameAttackPause += flameAttackPauseMax;
				}
				else flameAttackPause -= Time::deltaTime;
			}
		}
			break;

		case BossBehavior::ATTACK_CLOSE1:
		{
			if (hitCollider != nullptr)
			{
				hitCollider->active = false;
				hitCollider = nullptr;
			}
			else
			{
				manageStep();

				if (colliders[0].bone->getScale(animator.getCurrentTime()).y > 1.0f && !attacked)
				{
					attacked = true;
					hitCollider = ColliderPool::GetInstance()->createTriggerCollider(colliders[0].collider->radius * 3.0f, colliders[0].collider->point, this);
				}
			}
		}
		break;

		case BossBehavior::RUN:
		case BossBehavior::MOVE:
			modelMatrix[3][0] -= movementSpeed * Time::deltaTime * getMovementMod();
			manageStep();
			break; 

		case BossBehavior::MOVEBACK:
			modelMatrix[3][0] += movementSpeed * Time::deltaTime * getMovementMod();
			manageStep();
			break;

		/*case BossBehavior::RUN:
			modelMatrix[3][0] -= 1.5f * movementSpeed * Time::deltaTime * getMovementMod();
			manageStep();
			break;*/

		case BossBehavior::RUNBACK:
			modelMatrix[3][0] += 1.5f * movementSpeed * Time::deltaTime * getMovementMod();
			manageStep();
			break;

		default:
			break;
		}
	}

	void manageLegKnockback(Patapon* patapon)
	{
		if (patapon->getPosition().x > colliders[4].collider->point.x)
		{
			auto power = (patapon->getPosition().x - colliders[4].collider->point.x) + 0.06f;
			patapon->setKnockback(glm::vec2(power, power*0.33f));
			patapon->hit(1);
		}
	}

	inline float getMovementMod()
	{
		float offset = 5.0f;
		if (behavior == BossBehavior::MOVE)
			offset = -5.0f;

		auto x = animator.getCurrentAnimation()->GetDuration() / 2.0f;
		return std::max(fabs(std::sinf(((animator.getCurrentTime() + offset) / (x * 2.0f)) / (M_PI / x))), 0.42f); //-5.0f offset
	}

	inline void manageStep()
	{
		if (!stepped && colliders[4].bone->getScale(animator.getCurrentTime()).y > 1.0f)
		{
			stepped = true;
			Camera::earthQuake += 0.5f;

			BattleAudioMaster::GetInstance()->playDodongaStep();

			auto dirZ = RandomNumbers::getInstance().getDistribution("DodongaFireBreathZ", -0.02f, 0.02);
			auto pos = modelMatrix[3];
			pos.y += 0.5f;
			pos.z = 0.8f;
			if (behavior == BossBehavior::MOVEBACK || behavior == BossBehavior::RUNBACK)
				pos.x += scale * 0.3f;

			for (int i = 0; i < 2; i++)
			ParticleEmitter* particles = ParticleSystem::GetInstance()->createEmitter(pos, glm::normalize(glm::vec3(1.0f - 2.0f*i, 0.55f, dirZ)), 2.0f, 0.75f, 12.0f, 1.0f, 0.25f, 2.5f,
				-1.0, 0.0f, 0.0f, 64, 126, "dodongaStep", 30.0f, 0.5f);

		}
		else if (colliders[4].bone->getScale(animator.getCurrentTime()).y == 1.0f)
			stepped = false;
	}

	virtual void onCollision() override
	{
		if (active)
		{
			Collider* colid = nullptr;

			for (auto& col : colliders)
			{
				if (col.collider->collidedWith == nullptr)
					continue;
				else
				{
					colid = col.collider;
					break;
				}
			}

			if (colid == nullptr)
				return;

			auto damage = colid->collidedWith->parent->getDamage();
			HP -= damage;

			auto pos = colid->collidedWith->point;
			pos[0] += RandomNumbers::getInstance().getDistribution("hitNumberBoss", -0.125f, 0.125f);
			pos[2] += RandomNumbers::getInstance().getDistribution("hitNumberBoss", -0.125f, 0.125f);

			HitNumberPool::GetInstance()->createHitNumber(pos, damage);
			HitExplosionPool::GetInstance()->createHitExplosion(pos);

			colid->collidedWith = nullptr;

			if (HP <= 0)
			{
				clear();
				animator.PlayAnimation(&curAnimSet->animations[static_cast<int>(BossBehavior::SLEEP)]);//zmienic na change behavior
				active = false;
			}
		}
	}

	inline virtual int getDamage() override {
		return 40;
	}

	inline virtual float getKnockback() override {
		return 0.16f;
	}

	inline virtual void onTriggerCollision() override {
		BattleAudioMaster::GetInstance()->playDodongaHit();
	}

private:
	inline void onChangedAnimation()
	{
		clear();

		int i = 0;
		while (i < 100)
		{
			Bone* bone = animator.getCurrentAnimation()->FindBone("bCol" + std::to_string(i));
			if (bone == nullptr)
				break;

			Collider* col = ColliderPool::GetInstance()->createCollider(bone->getScale(0.0f).x / 100 * scale, modelMatrix[3], this); //skala.x to promien, skala.y to trigger
			colliders.emplace_back(bone, col);//glowa/szczeka
			i++;
		}
	}

	inline void clear()
	{
		for (auto col : colliders)
			col.collider->active = false;

		colliders.clear();
	}
};