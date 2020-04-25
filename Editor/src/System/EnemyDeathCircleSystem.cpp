#include "Systems/EnemyDeathCircleSystem.h"
#include "Components/Bullet.h"
#include "Components/Enemy.h"
#include "iw/engine/Time.h"

#include "IW/physics/Collision/SphereCollider.h";
#include "IW/physics/Dynamics/Rigidbody.h";

#include "Events/ActionEvents.h"
#include "iw/audio/AudioSpaceStudio.h"

EnemyDeathCircleSystem::EnemyDeathCircleSystem()
	: iw::System<iw::Transform, iw::Model, iw::CollisionObject, EnemyDeathCircle>("Enemy Death Circle")
{
	m_prefab.Radius   = 4.0f;
	m_prefab.FadeTime = 0.4f;
	m_prefab.Timer    = 0.0f;
}

int EnemyDeathCircleSystem::Initialize() {
	iw::Mesh sphere = Asset->Load<iw::Model>("Sphere")->GetMesh(0).MakeInstance();

	sphere.Material()->Set("albedo", iw::Color::From255(0, 195, 255, 64));
	sphere.Material()->SetTransparency(iw::Transparency::ADD);
	sphere.Material()->SetCastShadows(false);

	m_deathCircleModel.AddMesh(sphere);

	return 0;
}

void EnemyDeathCircleSystem::Update(
	iw::EntityComponentArray& view)
{
	for (auto entity : view) {
		auto [transform, model, object, circle] = entity.Components.Tie<Components>();

		float rate = circle->Radius / circle->FadeTime * iw::Time::DeltaTime();

		if (circle->Timer > 2 * circle->FadeTime) {
			transform->SetParent(nullptr);
			QueueDestroyEntity(entity.Index);
		}

		else if (circle->Timer < circle->FadeTime) {
			transform->Scale = iw::lerp(transform->Scale, iw::vector3(circle->Radius), rate);
			object->SetTrans(transform);
		}

		else {
			iw::Color* color = model->GetMesh(0).Material()->Get<iw::Color>("albedo");
			color->a = iw::lerp(color->a, 0.0f, rate);

			if (object->Col()) {
				object->SetCol(nullptr);
			}
		}

		circle->Timer += iw::Time::DeltaTime();
	}
}

bool EnemyDeathCircleSystem::On(
	iw::ActionEvent& e)
{
	if (e.Action == iw::val(Actions::SPAWN_ENEMY_DEATH)) {
		SpawnEnemyDeath& event = e.as<SpawnEnemyDeath>();
		iw::Transform* circle = SpawnDeathCircle(m_prefab, event.Position);
		circle->SetParent(event.Level);
	}

	return false;
}

iw::Transform* EnemyDeathCircleSystem::SpawnDeathCircle(
	EnemyDeathCircle prefab,
	iw::vector3 position)
{
	iw::Entity circle = Space->CreateEntity<iw::Transform, iw::Model, iw::SphereCollider, iw::CollisionObject, EnemyDeathCircle>();
	
	                         circle.SetComponent<EnemyDeathCircle>   (prefab);
	iw::Model*           m = circle.SetComponent<iw::Model>          (m_deathCircleModel);
	iw::Transform*       t = circle.SetComponent<iw::Transform>      (position, 0.0f);
	iw::SphereCollider*  s = circle.SetComponent<iw::SphereCollider> (iw::vector3::zero, 1);
	iw::CollisionObject* c = circle.SetComponent<iw::CollisionObject>();

	m->GetMesh(0).Material()->Set("albedo", iw::Color::From255(0, 195, 255, 64));

	c->SetCol(s);
	c->SetTrans(t);
	c->SetIsTrigger(true);

	Physics->AddCollisionObject(c);

	return t;
}