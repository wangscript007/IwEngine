#pragma once

#include "iw/engine/Events/ActionEvents.h"

#include "iw/math/vector2.h"
#include "iw/math/vector3.h"
#include "iw/common/Components/Transform.h"
#include "Components/Item.h"
#include "Components/Enemy.h"
#include "Components/Bullet.h"
#include "iw/entity/Entity.h"

enum class Actions
	: int
{
	GAME_STATE, DEV_CONSOLE,
	JUMP, RIGHT, FORWARD, USE,
	RESET_LEVEL, START_LEVEL, UNLOCK_LEVEL_DOOR, LOAD_NEXT_LEVEL, GOTO_NEXT_LEVEL, AT_NEXT_LEVEL,
	SPAWN_ENEMY_DEATH, SPAWN_ENEMY, SPAWN_BULLET, SPAWN_ITEM, SPAWN_NOTE, SPAWN_CONSUMABLE,
	GIVE_SCORE,
	SET_CAMERA_TARGET,
	CHARGE_KILL_ACTIVE, LONG_DASH_ACTIVE
};

struct DevConsoleEvent
	: iw::ToggleEvent
{
	DevConsoleEvent(
		bool active)
		: ToggleEvent(iw::val(Actions::DEV_CONSOLE), active)
	{}
};

enum GameState {
	NO_ACTION,
	PAUSED,
	RUNNING,
	SOFT_RUN,
	SOFT_PAUSE,
};

struct GameStateEvent
	: iw::SingleEvent
{
	GameState State;

	GameStateEvent(
		GameState state = GameState::NO_ACTION)
		: iw::SingleEvent(iw::val(Actions::GAME_STATE))
		, State(state)
	{}
};

struct JumpEvent
	: iw::ToggleEvent
{
	JumpEvent(
		bool active)
		: iw::ToggleEvent(iw::val(Actions::JUMP), active)
	{}
};

struct RightEvent
	: iw::ToggleEvent
{
	RightEvent(
		bool active)
		: iw::ToggleEvent(iw::val(Actions::RIGHT), active)
	{}
};

struct ForwardEvent
	: iw::ToggleEvent
{
	ForwardEvent(
		bool active)
		: iw::ToggleEvent(iw::val(Actions::FORWARD), active)
	{}
};

struct UseEvent
	: iw::SingleEvent
{
	UseEvent()
		: iw::SingleEvent(iw::val(Actions::USE))
	{}
};

struct ResetLevelEvent
	: iw::SingleEvent
{
	ResetLevelEvent()
		: iw::SingleEvent(iw::val(Actions::RESET_LEVEL))
	{}
};

struct StartLevelEvent
	: iw::SingleEvent
{
	std::string LevelName;
	bool CameraFollow;
	iw::vector3 PlayerPosition;

	StartLevelEvent(
		std::string levelName,
		bool cameraFollow,
		iw::vector3 playerPosition)
		: iw::SingleEvent(iw::val(Actions::START_LEVEL))
		, LevelName(levelName)
		, CameraFollow(cameraFollow)
		, PlayerPosition(playerPosition)
	{}
};


struct UnlockLevelDoorEvent
	: iw::SingleEvent
{
	UnlockLevelDoorEvent()
		: iw::SingleEvent(iw::val(Actions::UNLOCK_LEVEL_DOOR))
	{}
};

struct LoadNextLevelEvent
	: iw::SingleEvent
{
	std::string LevelName;
	iw::vector3 Position;
	bool GoBack;

	LoadNextLevelEvent()
		: iw::SingleEvent(iw::val(Actions::LOAD_NEXT_LEVEL))
		, GoBack(false)
	{}

	LoadNextLevelEvent(
		std::string levelName,
		iw::vector3 position = iw::vector3::zero,
		bool goback = false)
		: iw::SingleEvent(iw::val(Actions::LOAD_NEXT_LEVEL))
		, LevelName(levelName)
		, Position(position)
		, GoBack(goback)
	{}
};

struct GoToNextLevelEvent
	: iw::SingleEvent
{
	std::string LevelName;
	bool CameraFollow;
	iw::vector3 PlayerPosition;
	iw::vector3 CenterPosition;

	GoToNextLevelEvent(
		std::string levelName,
		bool cameraFollow,
		iw::vector3 playerPosition,
		iw::vector3 centerPosition)
		: iw::SingleEvent(iw::val(Actions::GOTO_NEXT_LEVEL))
		, LevelName(levelName)
		, CameraFollow(cameraFollow)
		, PlayerPosition(playerPosition)
		, CenterPosition(centerPosition)
	{}
};

struct AtNextLevelEvent
	: iw::SingleEvent
{
	AtNextLevelEvent()
		: iw::SingleEvent(iw::val(Actions::AT_NEXT_LEVEL))
	{}
};

struct SpawnEnemyDeath
	: iw::SingleEvent
{
	iw::vector3 Position;
	iw::Transform* Level;

	SpawnEnemyDeath(
		iw::vector3 position,
		iw::Transform* level)
		: iw::SingleEvent(iw::val(Actions::SPAWN_ENEMY_DEATH))
		, Position(position)
		, Level(level)
	{}
};

struct SpawnEnemyEvent
	: iw::SingleEvent
{
	Enemy Enemy;
	iw::vector3 Position;
	iw::vector3 Velocity;
	iw::Transform* Level;

	SpawnEnemyEvent(
		::Enemy enemy,
		iw::vector3 position,
		iw::vector3 velocity,
		iw::Transform* level)
		: iw::SingleEvent(iw::val(Actions::SPAWN_ENEMY))
		, Enemy(enemy)
		, Position(position)
		, Velocity(velocity)
		, Level(level)
	{}
};

struct SpawnBulletEvent
	: iw::SingleEvent
{
	Bullet Bullet;
	iw::vector3 Position;
	iw::quaternion Rotation;
	iw::Transform* Level;

	SpawnBulletEvent(
		::Bullet bullet,
		iw::vector3 position,
		iw::quaternion rotation,
		iw::Transform* level)
		: iw::SingleEvent(iw::val(Actions::SPAWN_BULLET))
		, Bullet(bullet)
		, Position(position)
		, Rotation(rotation)
		, Level(level)
	{}
};

struct SpawnItemEvent
	: iw::SingleEvent
{
	Item Item;
	iw::vector3 Position;
	iw::Transform* Level;

	SpawnItemEvent(
		::Item item,
		iw::vector3 position,
		iw::Transform* level)
		: iw::SingleEvent(iw::val(Actions::SPAWN_ITEM))
		, Item(item)
		, Position(position)
		, Level(level)
	{}
};

struct SpawnNoteEvent
	: iw::SingleEvent
{
	unsigned Index;

	SpawnNoteEvent(
		unsigned index)
		: iw::SingleEvent(iw::val(Actions::SPAWN_NOTE))
		, Index(index)
	{}
};

struct SpawnConsumableEvent
	: iw::SingleEvent
{
	unsigned Index;

	SpawnConsumableEvent(
		unsigned index)
		: iw::SingleEvent(iw::val(Actions::SPAWN_CONSUMABLE))
		, Index(index)
	{}
};

struct GiveScoreEvent
	: iw::SingleEvent
{
	iw::vector3 Position;
	int Score;
	bool HalfScore; // dont use score half potential score

	GiveScoreEvent(
		iw::vector3 position,
		int score,
		bool halfScore = false)
		: iw::SingleEvent(iw::val(Actions::GIVE_SCORE))
		, Position(position)
		, Score(score)
		, HalfScore(halfScore)
	{}
};

struct SetCameraTargetEvent
	: iw::SingleEvent
{
	iw::vector3 Target; // could be entity
	bool ResetY;

	SetCameraTargetEvent(
		iw::vector3 target,
		bool resetY = false)
		: iw::SingleEvent(iw::val(Actions::SET_CAMERA_TARGET))
		, Target(target)
		, ResetY(resetY)
	{}
};

struct ChargeKillEvent
	: iw::SingleEvent
{
	float Timer;

	ChargeKillEvent(
		float timer)
		: iw::SingleEvent(iw::val(Actions::CHARGE_KILL_ACTIVE))
		, Timer(timer)
	{}
};

struct LongDashEvent
	: iw::SingleEvent
{
	bool Active;

	LongDashEvent(
		bool active)
		: iw::SingleEvent(iw::val(Actions::LONG_DASH_ACTIVE))
		, Active(active)
	{}
};
