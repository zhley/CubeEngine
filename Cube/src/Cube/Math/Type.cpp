#include "Type.h"

namespace Cube {

	//===========================================================================
	Vec2 Vec2::operator+(const Vec2& other) {
		return {x + other.x, y + other.y};
	}

	Vec2 Vec2::operator-(const Vec2& other) {
		return {x - other.x, y - other.y};
	}

	float Vec2::operator*(const Vec2& other) {
	    return x * other.x + y * other.y;
	}

	float Vec2::cross(const Vec2& other) {
	    return x * other.y - y * other.x;
	}

	//===========================================================================
	Vec3 Vec3::operator+(const Vec3& other) {
		return {x + other.x, y + other.y, z + other.z};
	}

	Vec3 Vec3::operator-(const Vec3& other) {
		return {x - other.x, y - other.y, z - other.z};
	}

	float Vec3::operator*(const Vec3& other) {
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 Vec3::cross(const Vec3& other) {
		return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
	}

	//===========================================================================
	Vec4 Vec4::operator+(const Vec4& other) {
		return {x + other.x, y + other.y, z + other.z, w + other.w};
	}

	Vec4 Vec4::operator-(const Vec4& other) {
		return {x - other.x, y - other.y, z - other.z, w - other.w};
	}

	float Vec4::operator*(const Vec4& other) {
		return x * other.x + y * other.y + z * other.z + w + other.w;
	}

	//===========================================================================
	Mat2 Mat2::operator+(const Mat2& other) {
	    Mat2 res;
		for(int i = 0; i < 4; ++i) {
			res.data[i] = data[i] + other.data[i];
		}
		return res;
	}

	Mat2 Mat2::operator-(const Mat2& other) {
		Mat2 res;
		for(int i = 0; i < 4; ++i) {
			res.data[i] = data[i] - other.data[i];
		}
		return res;
	}

	Mat2 Mat2::operator*(const Mat2& other) {
		Mat2 res;
		for(int j = 0; j < 2; ++j) {
		    for(int i = 0; i < 2; ++i) {
				res.data[i + j * 2] = 0;
		        for(int k = 0; k < 2; ++k) {
		            res.data[i + j * 2] += data[i + k * 2] * other.data[k + j * 2];
		        }
		    }
		}
		return res;
	}

	Vec2 Mat2::operator*(const Vec2& other) {
	    return {data[0] * other.x + data[2] * other.y, data[1] * other.x + data[3] * other.y};
	}

	//===========================================================================
	Mat3 Mat3::operator+(const Mat3& other) {
		Mat3 res;
		for(int i = 0; i < 9; ++i) {
			res.data[i] = data[i] + other.data[i];
		}
		return res;
	}

	Mat3 Mat3::operator-(const Mat3& other) {
		Mat3 res;
		for(int i = 0; i < 9; ++i) {
			res.data[i] = data[i] - other.data[i];
		}
		return res;
	}

	Mat3 Mat3::operator*(const Mat3& other) {
		Mat3 res;
		for(int j = 0; j < 3; ++j) {
			for(int i = 0; i < 3; ++i) {
				res.data[i + j * 3] = 0;
				for(int k = 0; k < 3; ++k) {
					res.data[i + j * 3] += data[i + k * 3] * other.data[k + j * 3];
				}
			}
		}
		return res;
	}

	Vec3 Mat3::operator*(const Vec3& other) {
		return {data[0] * other.x + data[3] * other.y + data[6] * other.z, data[1] * other.x + data[4] * other.y + data[7] * other.z, data[2] * other.x + data[5] * other.y + data[8] * other.z};
	}

	//===========================================================================
	Mat4 Mat4::operator+(const Mat4& other) {
		Mat4 res;
		for(int i = 0; i < 16; ++i) {
			res.data[i] = data[i] + other.data[i];
		}
		return res;
	}

	Mat4 Mat4::operator-(const Mat4& other) {
		Mat4 res;
		for(int i = 0; i < 16; ++i) {
			res.data[i] = data[i] - other.data[i];
		}
		return res;
	}

	Mat4 Mat4::operator*(const Mat4& other) {
		Mat4 res;
		for(int j = 0; j < 4; ++j) {
			for(int i = 0; i < 4; ++i) {
				res.data[i + j * 4] = 0;
				for(int k = 0; k < 4; ++k) {
					res.data[i + j * 4] += data[i + k * 4] * other.data[k + j * 4];
				}
			}
		}
		return res;
	}

	Vec4 Mat4::operator*(const Vec4& other) {
		return {data[0] * other.x + data[4] * other.y + data[8] * other.z + data[12] * other.w, data[1] * other.x + data[5] * other.y + data[9] * other.z + data[13] * other.w,data[2] * other.x + data[6] * other.y + data[10] * other.z + data[14] * other.w,data[3] * other.x + data[7] * other.y + data[11] * other.z + data[15] * other.w};
	}
}