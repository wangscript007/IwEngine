#pragma once
#include "iw/reflection/Type.h"
#include "C:\dev\IwEngine\IwEngine\include\iw/math/vector3.h"

namespace iw {
namespace detail {
	template<>
	const Class* GetClass(ClassTag<iw::math::vector3>) {
		static Class c = Class("iw::math::vector3", 12, 3);
		c.fields[0] = {"x", GetType(TypeTag<float>()), 0};
		c.fields[1] = {"y", GetType(TypeTag<float>()), 4};
		c.fields[2] = {"z", GetType(TypeTag<float>()), 8};
		return &c;
	}
}
}
