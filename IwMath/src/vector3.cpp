#include "vector3.h"

namespace iwmath {
	const vector3 vector3::zero   = vector3(0, 0, 0);
	const vector3 vector3::one    = vector3(1, 1, 1);
	const vector3 vector3::unit_x = vector3(1, 0, 0);
	const vector3 vector3::unit_y = vector3(0, 1, 0);
	const vector3 vector3::unit_z = vector3(0, 0, 1);

	vector3::vector3()
		: x(0), y(0), z(0) {
	}

	vector3::vector3(float xyz)
		: x(xyz), y(xyz), z(xyz) {
	}

	vector3::vector3(float x, float y, float z)
		: x(x), y(y), z(z) {
	}

	float vector3::length() const {
		return sqrtf(x * x + y * y + z * z);
	}

	float vector3::length_squared() const {
		return x * x + y * y + z * z;
	}

	float vector3::length_fast() const {
		return 1 / inv_sqrt(x * x + y * y + z * z);
	}

	float vector3::dot(const vector3 & other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	vector3 vector3::cross(const vector3 & other) const {
		return vector3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x);
	}

	vector3 vector3::normalized() const {
		vector3 tmp = *this;
		tmp.normalize();
		return tmp;
	}

	vector3 vector3::normalized_fast() const {
		vector3 tmp = *this;
		tmp.normalize_fast();
		return tmp;
	}

	void vector3::normalize() {
		float scale = length();
		if (scale == 0) {
			return;
		}

		x /= scale;
		y /= scale;
		z /= scale;
	}

	void vector3::normalize_fast() {
		float scale = length_fast();
		if (scale == 0) {
			return;
		}

		x /= scale;
		y /= scale;
		z /= scale;
	}

	float& vector3::operator[](std::size_t index) {
		if (index == 0) return x;
		else if (index == 1) return y;
		else if (index == 2) return z;
		throw std::out_of_range("Index out of bounds");
	}

	vector3 vector3::operator+(const vector3& other) const {
		return vector3(x + other.x, y + other.y, z + other.z);
	}

	vector3 vector3::operator-(const vector3& other) const {
		return vector3(x - other.x, y - other.y, z - other.z);
	}

	vector3 vector3::operator*(const vector3& other) const {
		return vector3(x * other.x, y * other.y, z * other.z);
	}

	vector3 vector3::operator/(const vector3& other) const {
		return vector3(x / other.x, y / other.y, z / other.z);
	}

	vector3 vector3::operator+=(const vector3& other) {
		return *this = (*this) + other;
	}

	vector3 vector3::operator-=(const vector3& other) {
		return *this = (*this) - other;
	}

	vector3 vector3::operator*=(const vector3& other) {
		return *this = (*this) * other;
	}

	vector3 vector3::operator/=(const vector3& other) {
		return *this = (*this) / other;
	}

	vector3 vector3::operator+(float other) const {
		return vector3(x + other, y + other, z + other);
	}

	vector3 vector3::operator-(float other) const {
		return vector3(x - other, y - other, z - other);
	}

	vector3 vector3::operator*(float other) const {
		return vector3(x * other, y * other, z * other);
	}

	vector3 vector3::operator/(float other) const {
		return vector3(x / other, y / other, z / other);
	}

	vector3 vector3::operator+=(float other) {
		return *this = (*this) + other;
	}

	vector3 vector3::operator-=(float other) {
		return *this = (*this) - other;
	}

	vector3 vector3::operator*=(float other) {
		return *this = (*this) * other;
	}

	vector3 vector3::operator/=(float other) {
		return *this = (*this) / other;
	}

	vector3 vector3::operator-() const {
		return vector3(-x, -y, -z);
	}

	bool vector3::operator==(const vector3 & other) const {
		return almost_equal(x, other.x, 6) && almost_equal(y, other.y, 6) && almost_equal(z, other.z, 6);
	}

	bool vector3::operator!=(const vector3 & other) const {
		return !operator==(other);
	}

	std::ostream& iwmath::operator<<(std::ostream& ostream, const vector3& vector) {
		return ostream << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	}

	vector3 iwmath::operator+(float left, const vector3& right) {
		return right + left;
	}

	vector3 iwmath::operator*(float left, const vector3& right) {
		return right * left;
	}
}