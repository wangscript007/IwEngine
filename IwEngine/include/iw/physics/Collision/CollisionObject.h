#pragma once

#include "iw/common/Components/Transform.h"
#include "iw/physics/Collision/Collider.h"

namespace IW {
namespace Physics {
	constexpr size_t NOID = -1;

	class CollisionObject {
	protected:
		Transform* m_transform;     // Position, rotation, and scale
		Collider*  m_collider;      // Shape of the collision object (only 3d right now)
		size_t    m_id;             // Custom id for identifying colliders outside of the physics simulation

	public:
		CollisionObject();

		IWPHYSICS_API
		AABB Bounds() const;

		IWPHYSICS_API
		Transform* Trans() const;

		IWPHYSICS_API
		Collider* Col() const; // should be const probly also these are not needed if they are pointers but Ill try the get set thigs for now as a lil test because ive always dismissed it for the most part

		IWPHYSICS_API
		size_t Id() const;

		IWPHYSICS_API
		void SetTrans(
			Transform* transform);

		IWPHYSICS_API
		void SetCol(
			Collider* collider);

		IWPHYSICS_API
		void SetId(
			size_t id);
	};
}

	using namespace Physics;
}
