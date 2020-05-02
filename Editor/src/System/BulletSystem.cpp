#include "Systems/BulletSystem.h"
#include "Events/ActionEvents.h"
#include "Components/LevelDoor.h"
#include "Components/Player.h"
#include "Components/Enemy.h"
#include "iw/engine/Time.h"
#include <Components\DontDeleteBullets.h>

struct PlayerComponents {
	iw::Transform* Transform;
	Player* Player;
};

BulletSystem::BulletSystem(
	iw::Entity& player)
	: iw::System<iw::Transform, iw::Rigidbody, Bullet>("Bullet")
	, player(player)
{}

void BulletSystem::FixedUpdate(
	iw::EntityComponentArray& view)
{
	for (auto entity : view) {
		auto [transform, rigidbody, bullet] = entity.Components.Tie<Components>();

		switch (bullet->Type) {
			case SINE: {
				float speed = (sin(bullet->Time * 5) + 1) * 0.5f;
				rigidbody->SetVelocity(bullet->initialVelocity * speed);

				break;
			}
			case ORBIT: {
				iw::vector3 target = player.Find<iw::Transform>()->Position;
				rigidbody->ApplyForce((target - transform->Position) * 0.5f);

				break;
			}
			case SEEK: {
				iw::vector3 target = player.Find<iw::Transform>()->Position;
				rigidbody->SetVelocity((target - transform->Position).normalized() * bullet->Speed);

				break;
			}
		}

		bullet->Time += iw::Time::FixedTime();

		/*if (bullet->Time > 5.f) {
			QueueDestroyEntity(entity.Index);
			Physics->RemoveRigidbody(rigidbody);
		}*/
	}
}

bool BulletSystem::On(
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

	iw::Entity bullet;
	iw::Entity other;
	if (   a != iw::EntityHandle::Empty 
		&& a.Has<Bullet>()) 
	{
		bullet = a;
		other  = b;
	}

	else if (b != iw::EntityHandle::Empty
		  && b.Has<Bullet>())
	{
		bullet = b;
		other  = a;
	}

	if (   other != iw::EntityHandle::Empty
		&& (   other.Has<Bullet>() 
			|| other.Has<Enemy>()/*.Index() == bullet.Find<Bullet>()->enemyIndex*/
			|| other.Has<DontDeleteBullets>()
			|| other.Find<LevelDoor>()))
	{
		return false;
	}

	if (bullet != iw::EntityHandle::Empty) {
		bullet.Find<iw::Transform>()->SetParent(nullptr);
		QueueDestroyEntity(bullet.Index());
	}

	return false;
}
