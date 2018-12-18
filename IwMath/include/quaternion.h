#pragma once

#include <ostream>
#include "iwmath.h"
#include "vector3.h"
#include "vector4.h"

namespace iwmath {
	/**
	* Represents a rotation in 3D.
	*/
	struct IWMATH_API quaternion {
		static const quaternion identity;

		float x, y, z, w;

		quaternion();

		quaternion(
			float x,
			float y,
			float z,
			float w);

		quaternion(
			const vector3& xyz, float w);

		quaternion(
			const vector4& xyzw);

		/**
		* Returns the length of the quaternion.
		*/
		float length() const;

		/**
		* Returns the squared length of the quaternion.
		*	Use this for length comparisons.
		*/
		float length_squared() const;

		/**
		* Returns the length of the quaternion using the inv_sqrt function.
		*	This is much faster, and accurate enough in most cases.
		*/
		float length_fast() const;

		/**
		* Returns the normalized version of the quaternion.
		*/
		quaternion normalized() const;

		/**
		* Returns the normalized version of the quaternion using 
		*	the inv_sqrt function. This is much faster, 
		*	and accurate enough in most cases.
		*/
		quaternion normalized_fast() const;

		/**
		* Returns the inverted version of the quaternion.
		*/
		quaternion inverted() const;

		/**
		* Returns the conjugated version of the quaternion.
		*/
		quaternion conjugated() const;

		/**
		* Normalizes the quaternion.
		*/
		void normalize();

		/**
		* Normalizes the quaternion using the inv_sqrt function.
		*	This is much faster, and accurate enough in most cases.
		*/
		void normalize_fast();

		/**
		* Inverts the quaternion.
		*/
		void invert();

		/**
		* Conjugates the quaternion.
		*/
		void conjugate();

		/**
		* Returns the axis and the angle of the quaternion. 
		*	X, Y, Z is the axis, and W is the angle.
		*/
		vector4 axis_angle() const;

		/**
		* Returns the euler angle equivalent of the quaternion.
		*/
		vector3 euler_angles() const;

		///**
		//* Returns the rotation matrix of the quaternion as a matrix3.
		//*/
		//matrix3 matrix3() const;

		///**
		//* Returns the rotation matrix of the quaternion as a matrix4.
		//*/
		//matrix4 matrix4() const;

		quaternion operator+ (
			const quaternion& other) const;

		quaternion operator- (
			const quaternion& other) const;

		quaternion operator* (
			const quaternion& other) const;

		quaternion operator+=(
			const quaternion& other);

		quaternion operator-=(
			const quaternion& other);

		quaternion operator*=(
			const quaternion& other);

		quaternion operator* (
			float other) const;

		quaternion operator*=(
			float other);

		bool operator==(
			const quaternion& other) const;

		bool operator!=(
			const quaternion& other) const;

		/**
		* Returns a matrix with a specified rotation around an axis.
		*
		* @param axisAngle Axis and angle of rotation in one vector.
		*/
		static quaternion create_from_axis_angle(
			const vector4& axisAngle);

		/**
		* Returns a matrix with a specified rotation around an axis.
		*
		* @param axis Axis vector
		* @param angle Angle of rotation around the axis in radians.
		*/
		static quaternion create_from_axis_angle(
			const vector3& axis,
			float angle);

		/**
		* Returns a matrix with a specified rotation around an axis.
		*
		* @param x X component of the axis vector.
		* @param y Y component of the axis vector.
		* @param z X component of the axis vector.
		* @param angle Angle of rotation around the axis in radians.
		*/
		static quaternion create_from_axis_angle(
			float x,
			float y,
			float z,
			float angle);

		/**
		* Returns a quaternion from euler angles.
		*
		* @param angles Euler angles in radians.
		*/
		static quaternion create_from_euler_angles(
			const vector3& angles);

		/**
		* Returns a quaternion from euler angles.
		*
		* @param pitch Rotation around the x axis in radians.
		* @param yaw Rotation around the y axis in radians.
		* @param roll Rotation around the z axis in radians.
		*/
		static quaternion create_from_euler_angles(
			float x,
			float y,
			float z);

		friend IWMATH_API std::ostream& operator<<(
			std::ostream& stream,
			const quaternion& a);

		friend IWMATH_API vector3 operator*(
			const vector3& left,
			const quaternion& right);
	};
}