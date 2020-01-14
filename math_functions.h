#pragma once
#include "core.h"
#include <stdlib.h>
#include <math.h>


#define PI 3.141592653589793
#define FLT_MAX          3.402823466e+38F        // max value
#define FLT_MIN          1.175494351e-38F        // min normalized positive value 

struct vec3{
	float x, y, z;
	vec3& operator-=(const vec3& other){
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		return *this;
	}

	vec3& operator+=(const vec3& other){
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		return *this;
	}

	vec3& operator*=(float value){
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	vec3& operator/=(float value){
		ASSERT(value != 0.0f);
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
	bool operator==(const vec3& other){
		return (this->x == other.x && this->y == other.y && this->z == other.z);
	}
};

vec3 operator-(vec3 left, const vec3& right){
	left -= right;
	return left;
}
vec3 operator+(vec3 left, const vec3& right){
	left += right;
	return left;
}
vec3 operator*(vec3 left, float value){
	left *= value;
	return left;
}
vec3 operator*(float value, vec3 left){
	left *= value;
	return left;
}
vec3 operator/(vec3 left, float value){
	left /= value;
	return left;
}

// std::ostream& operator<<(std::ostream& stream, const vec3& v){
// 	std::cout << "{ " << v.x << ", " << v.y << ", " << v.z << "}";
// 	return stream;
// }

float dot(vec3 left, const vec3& right){
	return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);  
}

vec3 cross(const vec3& a, const vec3& b){
	vec3 result;
    result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

vec3 ComponentwiseMultiply(vec3 left, const vec3& right){
	return vec3{left.x *right.x, left.y * right.y, left.z * right.z};
}
float length2(const vec3& v){
	return v.x*v.x + v.y*v.y + v.z*v.z;
}
float length(const vec3& v){
	return sqrtf(length2(v));
}
vec3 normalize(vec3 v){
	float lengthSquared = length2(v);
	if (lengthSquared < 0.000001f){
        return v;
	}
    v /= sqrtf(lengthSquared);
	return v;
}

float LinearInterpolate(float A, float B, float t){
	ASSERT(t >= 0 && t <= 1.f)
	return A*(1-t) + B*t;
}
vec3 LinearInterpolate(const vec3& A, const vec3& B, float t){
	return {LinearInterpolate(A.x, B.x, t), LinearInterpolate(A.y, B.y, t), LinearInterpolate(A.z, B.z, t)};
}

struct CoordinateFrame{
	vec3 front;
	vec3 lateral;
	vec3 up;
};

CoordinateFrame CreateCoordinatFrame(vec3 z){
	CoordinateFrame result;
	if(z == vec3{0.f, 1.f, 0.f}){
		result.front = z;
		result.lateral = vec3{1, 0, 0};
		result.up = {0, 0, -1};
		return result;
	}

	vec3 x = normalize(cross({0.f, 1.f, 0.f}, z));
	vec3 y = normalize(cross(z, x));
	result.front = z;
	result.lateral = x;
	result.up = y;
	return result;
}


float RandomUnilateral(){
	return rand()/(float)RAND_MAX;
}

float RandomBilateral(){
	return rand()/(float)RAND_MAX * 2.0f - 1.0f;
}
