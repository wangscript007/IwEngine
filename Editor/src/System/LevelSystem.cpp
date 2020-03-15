#include "Systems/LevelSystem.h"
#include "Events/ActionEvents.h"
#include "Components/Player.h"
#include "iw/reflection/serialization/JsonSerializer.h"
#include "iw/graphics/Model.h"
#include "iw/physics/Dynamics/Rigidbody.h"
#include "iw/physics/Collision/SphereCollider.h"
#include "iw/engine/Time.h"

// should be generated
#include "iw/reflection/reflect/std/string.h"
#include "iw/reflection/reflect/std/vector.h"
#include "iw/reflect/math/vector2.h"
#include "iw/reflect/Components/Bullet.h"
#include "iw/reflect/Components/Enemy.h"
#include "iw/reflect/Components/Player.h"
#include "iw/reflect/Components/LevelDoor.h"
#include "iw/reflect/Components/Level.h"

LevelSystem::LevelSystem()
	: iw::System<iw::Transform, iw::Model, LevelDoor>("Level")
{
	//levels = {
	//	"assets/levels/forest/level1.json",
	//	"assets/levels/forest/level2.json",
	//	"assets/levels/forest/level3.json",
	//	"assets/levels/forest/level4.json",
	//	"assets/levels/forest/level5.json",
	//};

	//currentLevel = 0;

	currentLevelName = "levels/forest/level1.json";

	openColor   = iw::Color::From255(66, 201, 66, 127);
	closedColor = iw::Color::From255(201, 66, 66, 127);

	//Level level;
	//level.Enemies.push_back(Enemy{ EnemyType::SPIN, Bullet { LINE, 5 }, iw::Pi2 / 24.0f, 0.12f });
	//level.Positions.push_back(0);
	//level.Player = Player { 4.0f, 8 / 60.0f, .2f, 5 };
	//level.StageName = "";
	//level.Door = LevelDoor{
	//	LevelDoorState::LOCKED,
	//	iw::vector2(-5, 0),
	//	"next.json"
	//};

	//iw::JsonSerializer("assets/levels/working.json").Write(level);
}

int LevelSystem::Initialize() {
	Bus->push<ResetLevelEvent>();

	return 0;
}

void LevelSystem::Update(
	iw::EntityComponentArray& view)
{
	for (auto entity : view) {
		auto [transform, model, door] = entity.Components.Tie<Components>();

		transform->Rotation *= iw::quaternion::from_euler_angles(iw::Time::DeltaTime() * .1f);

		if (door->ColorTimer > 0) {
			door->ColorTimer -= iw::Time::DeltaTime();
			model->Meshes[0].Material->Set("albedo", iw::lerp<iw::vector4>(closedColor, openColor, 4 * (0.25f - door->ColorTimer)));
		}
	}
}

bool LevelSystem::On(
	iw::CollisionEvent& event)
{
	iw::Entity a = Space->FindEntity(event.ObjA);
	if (a == iw::EntityHandle::Empty) {
		a = Space->FindEntity<iw::Rigidbody>(event.ObjA);
	}

	iw::Entity b = Space->FindEntity(event.ObjB);
	if (b == iw::EntityHandle::Empty) {
		b = Space->FindEntity<iw::Rigidbody>(event.ObjB);
	}

	if (   a.Index() == iw::EntityHandle::Empty.Index
		|| b.Index() == iw::EntityHandle::Empty.Index)
	{
		return false;
	}

	iw::Entity doorEnt;
	if (   a.HasComponent<Player>()
		&& b.HasComponent<LevelDoor>())
	{
		doorEnt = b;
	}

	else if (b.HasComponent<Player>()
		&&   a.HasComponent<LevelDoor>())
	{
		doorEnt = a;
	}

	if (doorEnt.Index() != iw::EntityHandle::Empty.Index) {
		LevelDoor* door = doorEnt.FindComponent<LevelDoor>();
		if (door->State == LevelDoorState::OPEN) {
			Bus->push<NextLevelEvent>();
		}
	}

	return false;
}

bool LevelSystem::On(
	iw::ActionEvent& event)
{
	switch (event.Action) {
		case iw::val(Actions::NEXT_LEVEL): {
			currentLevelName = currentLevel.Door.NextLevel;
			//Task.Queue(iw::bind(LoadLevel, currentLevelName)), 5.0f);

			// no break
		}
		case iw::val(Actions::RESET_LEVEL): {
			LoadLevel(currentLevelName);
			break;
		}
		case iw::val(Actions::LOADED_LEVEL): {
			iw::Model* modelComp = currentDoor.FindComponent<iw::Model>();
			modelComp->Meshes[0].Material->Set("albedo", iw::Color::From255(201, 66, 66, 127));
			break;
		}
		case iw::val(Actions::OPEN_NEXT_LEVEL): {
			LevelDoor* doorComp = currentDoor.FindComponent<LevelDoor>();
			doorComp->State = LevelDoorState::OPEN;
			doorComp->ColorTimer = 0.25f;
			break;
		}
	}

	return false;
}

void LevelSystem::LoadLevel(
	std::string name)
{
	Space->Clear();

	iw::JsonSerializer("assets/" + name).Read(currentLevel);

	// Stage

	float scale = 2.0f;
	float scaleOutX = 1.6f;
	float scaleOutY = 1.8f;

	int x = 16 * scaleOutX;
	int y = 9 * scaleOutY;

	for (int l = 0; l < 1; l++) {
		for (int i = -x; i < x; i += 4) {
			LoadTree("models/forest/tree.dae", iw::Transform {
				iw::vector3(i, 0, y),
				scale,
				iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
					* iw::quaternion::from_axis_angle(iw::vector3::unit_y, rand() / (float)RAND_MAX * iw::Pi2)
			});
		}

		for (int i = -x; i < x; i += 4) {
			LoadTree("models/forest/tree.dae", iw::Transform {
				iw::vector3(i, 0, -y),
				scale,
				iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
					* iw::quaternion::from_axis_angle(iw::vector3::unit_y, rand() / (float)RAND_MAX * iw::Pi2)
			});
		}

		for (int i = -y; i <= y; i += 3) {
			LoadTree("models/forest/tree.dae", iw::Transform {
				iw::vector3(x, 0, i),
				scale,
				iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
					* iw::quaternion::from_axis_angle(iw::vector3::unit_y, rand() / (float)RAND_MAX * iw::Pi2)
			});
		}

		for (int i = -y + 3; i < y; i += 3) {
			LoadTree("models/forest/tree.dae", iw::Transform {
				iw::vector3(-x, 0, i),
				scale,
				iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
					* iw::quaternion::from_axis_angle(iw::vector3::unit_y, rand() / (float)RAND_MAX * iw::Pi2)
			});
		}
	
		scaleOutX = 1.6f;
		scaleOutY = 1.8f;

		x = 16 * scaleOutX;
		y = 9 * scaleOutY;
	}

	LoadFloor("models/forest/floor.dae", iw::Transform {
		iw::vector3(0, 0, 0),
		iw::vector3(32),
		iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
	});

	// Enemies

	for (size_t i = 0; i < currentLevel.Enemies.size(); i++) {
		iw::Entity enemy = Space->CreateEntity<iw::Transform, iw::Model, iw::SphereCollider, iw::CollisionObject, Enemy>();
		
		                          enemy.SetComponent<iw::Model>(*Asset->Load<iw::Model>("Tetrahedron"));
		                          enemy.SetComponent<Enemy>(currentLevel.Enemies[i]);
		iw::Transform*       te = enemy.SetComponent<iw::Transform>(iw::vector3(currentLevel.Positions[i].x, 1, currentLevel.Positions[i].y));
		iw::SphereCollider*  se = enemy.SetComponent<iw::SphereCollider>(iw::vector3::zero, 1.0f);
		iw::CollisionObject* re = enemy.SetComponent<iw::CollisionObject>();

		re->SetCol(se);
		re->SetTrans(te);

		Physics->AddCollisionObject(re);
	}

	// Player

	iw::vector3 playerPos = currentLevel.Door.NextLevelPosition;
	playerPos.z = playerPos.y;
	playerPos.y = 1.0f;

	iw::Entity player = Space->CreateEntity<iw::Transform, iw::Model, iw::SphereCollider, iw::Rigidbody, Player>();
	
	                         player.SetComponent<iw::Model>(*Asset->Load<iw::Model>("Player"));
	                         player.SetComponent<Player>(currentLevel.Player);
	iw::Transform*      tp = player.SetComponent<iw::Transform>(playerPos, iw::vector3(.75f));
	iw::SphereCollider* sp = player.SetComponent<iw::SphereCollider>(iw::vector3::zero, .75f);
	iw::Rigidbody*      rp = player.SetComponent<iw::Rigidbody>();

	rp->SetMass(1);
	rp->SetCol(sp);
	rp->SetTrans(tp);
	rp->SetStaticFriction(.1f);
	rp->SetDynamicFriction(.02f);

	rp->SetIsLocked(iw::vector3(0, 1, 0));
	rp->SetLock    (iw::vector3(0, 1, 0));

	Physics->AddRigidbody(rp);

	// Door

	currentDoor = Space->CreateEntity<iw::Transform, iw::Model, iw::SphereCollider, iw::CollisionObject, LevelDoor>();
	currentDoor.SetComponent<LevelDoor>(currentLevel.Door);

	iw::Model*           md = currentDoor.SetComponent<iw::Model>(*Asset->Load<iw::Model>("Door"));
	iw::Transform*       td = currentDoor.SetComponent<iw::Transform>(iw::vector3(20, 1, 0), 5.0f);
	iw::SphereCollider*  sd = currentDoor.SetComponent<iw::SphereCollider>(iw::vector3::zero, 5.0f);
	iw::CollisionObject* cd = currentDoor.SetComponent<iw::CollisionObject>();

	cd->SetCol(sd);
	cd->SetTrans(td);
	cd->SetIsTrigger(true);

	Physics->AddCollisionObject(cd);

	// Camera recreated in layer from \/

	Bus->push<LoadedLevelEvent>();
}

void LevelSystem::LoadTree(
	std::string name,
	iw::Transform transform)
{
	iw::ref<iw::Model> tree = Asset->Load<iw::Model>(name);

	if (tree->Meshes[0].VertexArray == nullptr) {
		for (size_t i = 0; i < tree->MeshCount; i++) {
			iw::ref<iw::Material>& mat = tree->Meshes[i].Material;

			mat->SetShader(Asset->Load<iw::Shader>("shaders/pbr.shader"));
			mat->SetTexture("shadowMap",  Asset->Load<iw::Texture>("SunShadowMap"));   // shouldnt be part of material
			mat->SetTexture("shadowMap2", Asset->Load<iw::Texture>("LightShadowMap")); // shouldnt be part of material
			mat->Initialize(Renderer->Device);

			mat->Set("roughness", 0.7f);
			mat->Set("metallic", 0.0f);

			tree->Meshes[i].GenTangents();
			tree->Meshes[i].Initialize(Renderer->Device);
		}
	}

	iw::ref<iw::Texture> leavesAlpha = Asset->Load<iw::Texture>("textures/forest/tree/leaves/alpha.jpg");
	leavesAlpha->Initialize(Renderer->Device);

	tree->Meshes[1].Material->SetTexture("alphaMaskMap", leavesAlpha);

	iw::Entity ent = Space->CreateEntity<iw::Transform, iw::Model>();
	ent.SetComponent<iw::Transform>(transform);
	ent.SetComponent<iw::Model>(*tree);
}

void LevelSystem::LoadFloor(
	std::string name,
	iw::Transform transform)
{
	iw::ref<iw::Model> floor = Asset->Load<iw::Model>(name);

	if (floor->Meshes[0].VertexArray == nullptr) {
		for (size_t i = 0; i < floor->MeshCount; i++) {
			iw::ref<iw::Material>& mat = floor->Meshes[i].Material;

			mat->SetShader(Asset->Load<iw::Shader>("shaders/pbr.shader"));
			mat->SetTexture("shadowMap",  Asset->Load<iw::Texture>("SunShadowMap"));   // shouldnt be part of material
			mat->SetTexture("shadowMap2", Asset->Load<iw::Texture>("LightShadowMap")); // shouldnt be part of material
			mat->Initialize(Renderer->Device);

			mat->Set("roughness", 0.9f);
			mat->Set("metallic", 0.1f);

			floor->Meshes[i].GenTangents();
			floor->Meshes[i].Initialize(Renderer->Device);
		}
	}

	iw::Entity ent = Space->CreateEntity<iw::Transform, iw::Model>();
	ent.SetComponent<iw::Transform>(transform);
	ent.SetComponent<iw::Model>(*floor);
}
