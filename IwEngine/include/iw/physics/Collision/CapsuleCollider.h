#pragma once

#include "Collider.h"

#include "iw/engine/Time.h"

namespace iw {
namespace Physics {
namespace impl {
	template<
		typename V>
	struct REFLECT CapsuleCollider
		: Collider<V>
	{
		REFLECT V Position;
		REFLECT V Offset;
		REFLECT float Radius;

		CapsuleCollider()
			: Collider<V>(ColliderType::CAPSULE)
			, Position(0.0f)
			, Offset(0.0f, 1.0f, 0.0f)
			, Radius(1.0f)
		{}

		CapsuleCollider(
			V position,
			V offset,
			float radius)
			: Collider<V>(ColliderType::CAPSULE)
			, Position(position)
			, Offset(offset)
			, Radius(radius)
		{}

		const AABB<V>& Bounds() override {
			if (m_outdated) {
				m_bounds = AABB<V>(
					Position + Position.normalized() * Radius,
					Position + Offset + Offset.normalized() * Radius
				);

				m_outdated = false;
			}

			return m_bounds;
		}

		Transform Trans() const override {
			Transform transform;
			transform.Position = (Position + Offset) / 2;
			transform.Scale    = vector3(Radius, Offset.length(), Radius);
			transform.Rotation = quaternion::from_look_at(Position, Position + Offset);
			transform.Rotation *= quaternion::from_axis_angle(transform.Right(), Pi / 2);

			return transform;
		}

		ManifoldPoints TestCollision(
			const Transform* transform,
			const Collider<V>* collider,
			const Transform* colliderTransform) const override
		{
			return collider->TestCollision(colliderTransform, this, transform);
		}

		IWPHYSICS_API
		ManifoldPoints TestCollision(
			const Transform* transform,
			const SphereCollider<V>* sphere,
			const Transform* sphereTransform) const override;

		IWPHYSICS_API
		ManifoldPoints TestCollision(
			const Transform* transform,
			const CapsuleCollider<V>* capsule,
			const Transform* capsuleTransform) const override;

		IWPHYSICS_API
		ManifoldPoints TestCollision(
			const Transform* transform,
			const PlaneCollider<V>* plane,
			const Transform* planeTransform) const override;
	};
}

	using CapsuleCollider2 = impl::CapsuleCollider<iw::vector2>;
	using CapsuleCollider  = impl::CapsuleCollider<iw::vector3>;
	using CapsuleCollider4 = impl::CapsuleCollider<iw::vector4>;
}

	using namespace Physics;
}
