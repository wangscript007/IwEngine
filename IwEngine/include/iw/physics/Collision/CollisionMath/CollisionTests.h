#pragma once

#include "iw/physics/IwPhysics.h"
#include "iw/physics/Collision/SphereCollider.h"

namespace IW {
namespace Physics {
namespace algo {


	//template<
	//	typename V>
	//bool TestCollision(
	//	const SphereCollider &left,
	//	const SphereCollider &right,
	//	V *resolve = nullptr)
	//{
	//	double r = left.Radius + right.Radius;
	//	V d = left.Center - right.Center;
	//	if (d.length() < r) {
	//		if(resolve != nullptr) *resolve = (d / d.length()) * (r - d.length());
	//		return true;
	//	}
	//	return false;
	//}

	//bool TestCollision(
	//	const SphereCollider& left,
	//	const SphereCollider2& right,
	//	iw::vector3* resolve = nullptr)
	//{
	//	return false;
	//}

	//bool TestCollision(
	//	const BoxCollider<iw::vector2>& left,
	//	const BoxCollider<iw::vector2>& right,
	//	iw::vector2 *resolve = nullptr)
	//{
	//	return false;
	//}
}
}

	using namespace Physics;
}
