#pragma once

#include "iw/engine/System.h"
#include "iw/common/Components/Transform.h"
#include "Components/LevelDoor.h"
#include "Components/Level.h"
#include "iw/graphics/Model.h"

#include "iw/engine/Events/Seq/EventSequence.h"
#include "iw/physics/Collision/CollisionObject.h"

class LevelSystem
	: public iw::System<iw::CollisionObject, iw::Model, LevelDoor>
{
public:
	struct Components {
		iw::CollisionObject* Transform;
		iw::Model* Model;
		LevelDoor* Door;
	};
private:
	std::string currentLevelName;
	Level currentLevel;

	iw::Entity& playerEntity;
	iw::Scene* scene;

	iw::Entity levelEntity;
	iw::Entity nextLevelEntity;
	bool transition;

	iw::Entity levelDoor;
	iw::Entity nextLevelDoor;
	iw::Color openColor;
	iw::Color closedColor;

	iw::EventSequence sequence; // For cut scenes, prob shouldnt be here
	iw::Entity firstEnemy;
	iw::Entity otherGuy;

public:
	LevelSystem(
		iw::Entity& player,
		iw::Scene* scene);

	int Initialize() override;

	void Update(
		iw::EntityComponentArray& view) override;

	bool On(
		iw::CollisionEvent& event);

	bool On(
		iw::ActionEvent& event);

	iw::Entity LoadLevel(
		std::string name);

	iw::Entity& GetLevel() { return levelEntity; }
private:
	void DestroyAll(
		iw::Transform* transform);
};
