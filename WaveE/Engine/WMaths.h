#pragma once

#include <DirectXMath.h>

namespace WaveE
{
	namespace wma
	{
		struct vec2
		{
			// This is undefined behavior, but makes the use of the struct a lot more intuitive
			union
			{
				struct
				{
					float x;
					float y;
				};
				DirectX::XMFLOAT2 float2;
			};

			vec2() = default;

			vec2(float _x, float _y);

			vec2(float val);

			// Operators
			vec2 operator-() const;

			vec2 operator+(const vec2& rhs) const;

			vec2 operator-(const vec2& rhs) const;

			vec2 operator*(float scalar) const;

			vec2& operator+=(const vec2& rhs);

			vec2& operator-=(const vec2& rhs);

			vec2& operator*=(float scalar);

			float& operator[](int index);

			const float& operator[](int index) const;

			// Other useful functions
			float length() const;

			vec2 normalize() const;
		};

		struct vec3
		{
			// This is undefined behavior, but makes the use of the struct a lot more intuitive
			union
			{
				struct
				{
					float x;
					float y;
					float z;
				};
				struct {
					vec2 xy;
				};
				DirectX::XMFLOAT3 float3;
			};

			vec3() = default;

			vec3(float _x, float _y, float _z);

			vec3(float val);

			vec3(const vec2& float2, float _z = 0);

			// Operators
			vec3 operator-() const;

			vec3 operator+(const vec3& rhs) const;

			vec3 operator-(const vec3& rhs) const;

			vec3 operator*(float scalar) const;

			vec3& operator+=(const vec3& rhs);

			vec3& operator-=(const vec3& rhs);

			vec3& operator*=(float scalar);

			float& operator[](int index);

			const float& operator[](int index) const;

			// Other useful functions
			float length() const;

			vec3 normalize() const;
		};

		struct vec4
		{
			// This is undefined behavior, but makes the use of the struct a lot more intuitive
			union
			{
				struct
				{
					float x;
					float y;
					float z;
					float w;
				};
				struct 
				{
					vec3 xyz;
				};
				struct 
				{
					vec2 xy;
				};
				DirectX::XMFLOAT4 float4;
			};

			vec4() = default;

			vec4(float _x, float _y, float _z, float _w);

			vec4(float val);

			vec4(const vec3& float3, float _w = 0);

			vec4(const vec2& float2, float _z = 0, float _w = 0);

			// Operators
			vec4 operator-() const;

			vec4 operator+(const vec4& rhs) const;

			vec4 operator-(const vec4& rhs) const;

			vec4 operator*(float scalar) const;

			vec4& operator+=(const vec4& rhs);

			vec4& operator-=(const vec4& rhs);

			vec4& operator*=(float scalar);

			float& operator[](int index);

			const float& operator[](int index) const;

			// Other useful functions
			float length() const;

			vec4 normalize() const;
		};

		struct mat3
		{
			// This is undefined behavior, but makes the use of the struct a lot more intuitive
			union
			{
				DirectX::XMFLOAT3X3 data;
				vec3 rowData[3];
			};

			mat3();

			mat3(const DirectX::XMMATRIX& mat);

			mat3(const vec3 row0, const vec3 row1, const vec3 row2, const vec3 row3);

			// Operators
			mat3 operator*(const mat3& rhs) const;

			vec3 operator*(const vec3& rhs) const;

			const vec3& operator[](int index) const;

			vec3& operator[](int index);

			// Other useful functions
			static mat3 identity();

			void inverse();

			void translate(const vec3& translation);

			void rotate(float angle, const vec3& axis);

			void scale(const vec3& scale);

			void transpose();
		};

		struct mat4
		{
			// This is undefined behavior, but makes the use of the struct a lot more intuitive
			union
			{
				DirectX::XMFLOAT4X4 data;
				vec4 rowData[4];
			};

			mat4();

			mat4(const DirectX::XMMATRIX& mat);

			mat4(const vec4 row0, const vec4 row1, const vec4 row2, const vec4 row3);

			// Operators
			mat4 operator*(const mat4& rhs) const;

			vec3 operator*(const vec3& rhs) const;

			const vec4& operator[](int index) const;

			vec4& operator[](int index);

			// Other useful functions
			static mat4 identity();

			void inverse();

			void translate(const vec3& translation);

			void rotate(float angle, const vec3& axis);

			void scale(const vec3& scale);

			void transpose();
		};

		vec4 normalize(const vec4& vector);

		vec3 normalize(const vec3& vector);

		vec2 normalize(const vec2& vector);

		mat4 inverse(const mat4& matrix);

		mat3 inverse(const mat3& matrix);

		mat4 transpose(const mat4& matrix);

		mat3 transpose(const mat3& matrix);

		vec3 cross(const vec3& lhs, const vec3& rhs);

		float dot(const vec2& lhs, const vec2& rhs);

		float dot(const vec3& lhs, const vec3& rhs);

		float dot(const vec4& lhs, const vec4& rhs);

		vec4 operator*(float scalar, const vec4& vec);

		vec3 operator*(float scalar, const vec3& vec);

		vec2 operator*(float scalar, const vec2& vec);

		mat4 rotate(const mat4& matrix, float angle, vec3 axis);

		mat3 rotate(const mat3& matrix, float angle, vec3 axis);

		mat4 scale(const mat4& matrix, vec3 scaleVector);

		mat3 scale(const mat3& matrix, vec3 scaleVector);

		mat4 translate(const mat4& matrix, vec3 translateVector);

		mat3 translate(const mat3& matrix, vec3 translateVector);

		mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up);

		mat4 perspective(float fovY, float aspect, float nearZ, float farZ);

		mat4 orthographic(float width, float height, float nearZ, float farZ);

		mat4 rotation(float pitch, float yaw, float roll = 0);

		// Convert radians to degrees
		float degrees(float radians);

		// Convert degrees to radians
		float radians(float degrees);
	};
}

