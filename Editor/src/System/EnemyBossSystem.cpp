#include "Systems/EnemyBossSystem.h"

#include "iw/engine/Time.h"
#include "iw/graphics/Model.h"
#include "iw/input/Devices/Keyboard.h"

#include "iw/physics/Collision/SphereCollider.h";
#include "iw/physics/Dynamics/Rigidbody.h";
#include "iw/physics/AABB.h"

#include "iw/audio/AudioSpaceStudio.h"

#include "Components/Bullet.h"
#include "Components/Player.h"
#include "Components/LevelDoor.h"
#include "Components/DontDeleteBullets.h"
#include "Components/EnemyDeathCircle.h"

#include "Events/ActionEvents.h"

#include "Systems/EnemySystem.h"

EnemyBossSystem::EnemyBossSystem(
	iw::Entity& player,
	const EnemySystem* enemySystem)
	: iw::SystemBase("Enemy Boss")
	, m_player(player)
	, m_enemySystem(enemySystem)
	, m_musicInstance(-1)
{}

int EnemyBossSystem::Initialize() {
	m_actions.push_back(&EnemyBossSystem::action_forest_spin);
	m_actions.push_back(&EnemyBossSystem::action_forest_wave);
	m_actions.push_back(&EnemyBossSystem::action_forest_wave_gap);
	m_actions.push_back(&EnemyBossSystem::action_forest_enemy_fling);

	m_actions.push_back(&EnemyBossSystem::action_canyon_front_at_player);
	m_actions.push_back(&EnemyBossSystem::action_canyon_side_seek);
	m_actions.push_back(&EnemyBossSystem::action_canyon_back_orbit);
	
	m_conditions.push_back(&EnemyBossSystem::condition_enemy_count_or_just_hit);

	return 0;
}

void EnemyBossSystem::Update() {
	auto bosses = Space->Query<iw::Transform, Enemy, EnemyBoss>();

	bosses.Each([&](
		iw::EntityHandle entity,
		iw::Transform*   transform,
		Enemy*           enemy,
		EnemyBoss*       boss)
	{
		if (boss->JustSpawned) {
			if (m_musicInstance != -1) {
				Audio->AsStudio()->StopInstance(m_musicInstance);
				Audio->AsStudio()->StartInstance(m_musicInstance);
			}

			else {
				m_musicInstance = Audio->AsStudio()->CreateInstance("Music/boss", false);
			}

			boss->JustSpawned = false;
			boss->CurrentAction = boss->FirstAction;
		}

		Audio->AsStudio()->SetInstanceParameter(m_musicInstance, "BossHealth", enemy->Health);

		if (enemy->Health == 0) {
			transform->SetParent(nullptr);
			Space->QueueEntity(entity, iw::func_Destroy);
			return;
		}


		if (enemy->ChargeTime > enemy->Timer) {
			return;
		}

		if (boss->CurrentAction == -1) {
			int index = -1;

			for (int i = 0; i < boss->Actions.size(); i++) {
				Action& action = boss->Actions.at(i);
				if (action.Condition != -1) {
					bool pickme = std::invoke(m_conditions.at(action.Condition), this, transform, enemy);
					if (pickme) {
						index = i;
						break;
					}
				}
			}

			if (index == -1) {
				index = iw::randi(boss->Actions.size() - 1);
			}

			boss->CurrentAction = index;
		}
		
		Action& action = boss->Actions.at(boss->CurrentAction); 

		if (action.Time > enemy->Timer - enemy->ChargeTime) {
			if (boss->ActionTimer == 0.0f) {
				std::invoke(m_actions.at(action.Index), this, transform, enemy);
			}

			boss->ActionTimer += iw::Time::DeltaTimeScaled();

			if (boss->ActionTimer > action.Delay) {
				boss->ActionTimer = 0.0f;
			}
		}

		else if (action.Time < enemy->Timer - enemy->ChargeTime) {
			enemy->Timer = 0;
			boss->ActionTimer = 0;
			boss->CurrentAction = -1;
		}
	});
}

void EnemyBossSystem::action_forest_spin(
	iw::Transform* transform,
	Enemy* enemy)
{
	float rot = enemy->Timer * iw::Pi2 * 2;
	iw::quaternion offset = iw::quaternion::from_euler_angles(0, rot, 0);

	Bus->push<SpawnBulletEvent>(
		enemy->Bullet,
		transform->Position,
		transform->Rotation.inverted() * offset,
		transform->Parent()
	);
}

void EnemyBossSystem::action_forest_wave(
	iw::Transform* transform,
	Enemy* enemy)
{
	float rot = iw::Pi + iw::hPi * 0.5f * cos(enemy->Timer);

	int count = roundf(iw::Pi2 / enemy->Speed);
	int dontShoot = count * 0.5f;

	for (int i = 0; i <= count; i++) {
		if (   i != dontShoot
			&& i != dontShoot - 1
			&& i != dontShoot + 1)
		{
			iw::quaternion offset = iw::quaternion::from_euler_angles(0, -rot + enemy->Speed * i, 0);

			Bus->push<SpawnBulletEvent>(
				enemy->Bullet,
				transform->Position,
				transform->Rotation.inverted() * offset,
				transform->Parent()
			);
		}
	}
}

void EnemyBossSystem::action_forest_wave_gap(
	iw::Transform* transform,
	Enemy* enemy)
{
	int count = roundf(iw::Pi2 / enemy->Speed);
	float rot = iw::Pi + iw::hPi * 0.5f * cos(enemy->Timer);

	for (int i = 0; i < count; i++) {
		iw::quaternion offset = iw::quaternion::from_euler_angles(0, -rot + enemy->Speed * i, 0);

		Bus->push<SpawnBulletEvent>(
			enemy->Bullet,
			transform->Position,
			transform->Rotation.inverted() * offset,
			transform->Parent()
		);
	}
}

void EnemyBossSystem::action_forest_enemy_fling(
	iw::Transform* transform,
	Enemy* enemy)
{
	float rot = iw::Time::TotalTime() * iw::Pi2 * 3;

	iw::quaternion offset = transform->Rotation.inverted()
		* iw::quaternion::from_euler_angles(0, rot, 0);

	Enemy child{};
	child.Type = EnemyType::SPIN;
	child.Bullet = enemy->Bullet;
	child.Speed = 0.2617994;
	child.FireTime = 0.120000;
	child.ChargeTime = 0.000000;
	child.HasShot = false;
	child.JustHit = false;

	iw::vector3 position = transform->Position + iw::vector3(sqrt(2), 1, 0) * offset;
	iw::vector3 velocity = transform->Forward() * offset;

	velocity *= 7.0f + iw::randf() * 2;
	velocity.y = 10.0f;

	Bus->push<SpawnEnemyEvent>(
		child,
		position,
		velocity,
		transform->Parent());
}

void EnemyBossSystem::action_canyon_front_at_player(
	iw::Transform* transform,
	Enemy* enemy)
{
	int count = roundf(iw::hPi / enemy->Speed);
	float rot = -iw::hPi * 0.5f;

	for (int i = 0; i < count; i++) {
		iw::quaternion offset = iw::quaternion::from_euler_angles(0, rot + i * enemy->Speed, 0);

		Bus->push<SpawnBulletEvent>(
			enemy->Bullet,
			transform->Position,
			transform->Rotation.inverted() * offset,
			transform->Parent()
		);
	}

	if (enemy->Health <= 2 && iw::randf() > 0.6) {
		action_canyon_side_seek(transform, enemy);
	}
}

void EnemyBossSystem::action_canyon_side_seek(
	iw::Transform* transform,
	Enemy* enemy)
{
	int count = 4;
	float rot = iw::hPi * 0.5f;

	for (int i = 0; i < count; i++) {
		iw::quaternion offset = iw::quaternion::from_euler_angles(0, rot + i * iw::hPi / count, 0);

		Bus->push<SpawnBulletEvent>(
			m_seekBullet,
			transform->Position,
			transform->Rotation.inverted() * offset,
			transform->Parent()
		);
	}

	for (int i = 0; i < count; i++) {
		iw::quaternion offset = iw::quaternion::from_euler_angles(0, -rot - i * iw::hPi / count, 0);

		Bus->push<SpawnBulletEvent>(
			m_seekBullet,
			transform->Position,
			transform->Rotation.inverted() * offset,
			transform->Parent()
		);
	}

	if (enemy->Health == 1) {
		action_canyon_back_orbit(transform, enemy);
	}
}

void EnemyBossSystem::action_canyon_back_orbit(
	iw::Transform* transform,
	Enemy* enemy)
{
	int count = roundf(iw::hPi / enemy->Speed);
	float rot = iw::Pi + -iw::hPi * 0.5f;

	for (int i = 0; i < count; i++) {
		iw::quaternion offset = iw::quaternion::from_euler_angles(0, rot + i * enemy->Speed, 0);

		Bus->push<SpawnBulletEvent>(
			m_orbitBullet,
			transform->Position,
			transform->Rotation.inverted() * offset,
			transform->Parent()
		);
	}
}

bool EnemyBossSystem::condition_enemy_count_or_just_hit(
	iw::Transform* transform,
	Enemy* enemy)
{
	return m_enemySystem->GetEnemyCount() > 4 || enemy->JustHit;
}

bool EnemyBossSystem::condition_enemy_health_2(
	iw::Transform* transform,
	Enemy* enemy)
{
	return enemy->Health == 2;
}

bool EnemyBossSystem::condition_enemy_health_1(
	iw::Transform* transform,
	Enemy* enemy)
{
	return enemy->Health == 1;
}
