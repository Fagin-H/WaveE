#include "stdafx.h"
#include "WMaths.h"

namespace WaveE
{
	namespace wma
	{

		vec2::vec2(float _x, float _y)
		{
			x = _x;
			y = _y;
		}

		vec2::vec2(float val)
		{
			x = val;
			y = val;
		}

		vec2 vec2::operator-() const
		{
			return vec2(-x, -y);
		}

		vec2 vec2::operator-(const vec2& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat2(&float2);
			XMVECTOR rhsVec = XMLoadFloat2(&rhs.float2);
			XMVECTOR resultVec = XMVectorSubtract(lhsVec, rhsVec);
			vec2 result;
			XMStoreFloat2(&result.float2, resultVec);
			return result;
		}

		vec2 vec2::operator*(float scalar) const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat2(&float2);
			XMVECTOR resultVec = XMVectorScale(vec, scalar);
			vec2 result;
			XMStoreFloat2(&result.float2, resultVec);
			return result;
		}

		vec2& vec2::operator*=(float scalar)
		{
			*this = *this * scalar;
			return *this;
		}

		float& vec2::operator[](int index)
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			default: throw std::out_of_range("Index out of range");
			}
		}

		const float& vec2::operator[](int index) const
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			default: throw std::out_of_range("Index out of range");
			}
		}

		float vec2::length() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat2(&float2);
			return XMVectorGetX(XMVector2Length(vec));
		}

		vec2 vec2::normalize() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat2(&float2);
			XMVECTOR resultVec = XMVector2Normalize(vec);
			vec2 result;
			XMStoreFloat2(&result.float2, resultVec);
			return result;
		}

		vec2& vec2::operator-=(const vec2& rhs)
		{
			*this = *this - rhs;
			return *this;
		}

		vec2& vec2::operator+=(const vec2& rhs)
		{
			*this = *this + rhs;
			return *this;
		}

		vec2 vec2::operator+(const vec2& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat2(&float2);
			XMVECTOR rhsVec = XMLoadFloat2(&rhs.float2);
			XMVECTOR resultVec = XMVectorAdd(lhsVec, rhsVec);
			vec2 result;
			XMStoreFloat2(&result.float2, resultVec);
			return result;
		}

		vec3::vec3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		vec3::vec3(float val)
		{
			x = val;
			y = val;
			z = val;
		}

		vec3::vec3(const vec2& float2, float _z /*= 0*/)
		{
			x = float2.x;
			y = float2.y;
			z = _z;
		}

		vec3& vec3::operator*=(float scalar)
		{
			*this = *this * scalar;
			return *this;
		}

		vec3 vec3::operator*(float scalar) const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat3(&float3);
			XMVECTOR resultVec = XMVectorScale(vec, scalar);
			vec3 result;
			XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		vec3& vec3::operator+=(const vec3& rhs)
		{
			*this = *this + rhs;
			return *this;
		}

		vec3 vec3::operator+(const vec3& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat3(&float3);
			XMVECTOR rhsVec = XMLoadFloat3(&rhs.float3);
			XMVECTOR resultVec = XMVectorAdd(lhsVec, rhsVec);
			vec3 result;
			XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		vec3& vec3::operator-=(const vec3& rhs)
		{
			*this = *this - rhs;
			return *this;
		}

		vec3 vec3::operator-(const vec3& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat3(&float3);
			XMVECTOR rhsVec = XMLoadFloat3(&rhs.float3);
			XMVECTOR resultVec = XMVectorSubtract(lhsVec, rhsVec);
			vec3 result;
			XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		vec3 vec3::operator-() const
		{
			return vec3(-x, -y, -z);
		}

		const float& vec3::operator[](int index) const
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: throw std::out_of_range("Index out of range");
			}
		}

		float& vec3::operator[](int index)
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: throw std::out_of_range("Index out of range");
			}
		}

		float vec3::length() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat3(&float3);
			return XMVectorGetX(XMVector3Length(vec));
		}

		vec3 vec3::normalize() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat3(&float3);
			XMVECTOR resultVec = XMVector3Normalize(vec);
			vec3 result;
			XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		vec4& vec4::operator*=(float scalar)
		{
			*this = *this * scalar;
			return *this;
		}

		vec4 vec4::operator*(float scalar) const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat4(&float4);
			XMVECTOR resultVec = XMVectorScale(vec, scalar);
			vec4 result;
			XMStoreFloat4(&result.float4, resultVec);
			return result;
		}

		vec4& vec4::operator+=(const vec4& rhs)
		{
			*this = *this + rhs;
			return *this;
		}

		vec4 vec4::operator+(const vec4& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat4(&float4);
			XMVECTOR rhsVec = XMLoadFloat4(&rhs.float4);
			XMVECTOR resultVec = XMVectorAdd(lhsVec, rhsVec);
			vec4 result;
			XMStoreFloat4(&result.float4, resultVec);
			return result;
		}

		vec4::vec4(const vec2& float2, float _z /*= 0*/, float _w /*= 0*/)
		{
			x = float2.x;
			y = float2.y;
			z = _z;
			w = _w;
		}

		vec4::vec4(const vec3& float3, float _w /*= 0*/)
		{
			x = float3.x;
			y = float3.y;
			z = float3.z;
			w = _w;
		}

		vec4::vec4(float val)
		{
			x = val;
			y = val;
			z = val;
			w = val;
		}

		vec4::vec4(float _x, float _y, float _z, float _w)
		{
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}

		vec4& vec4::operator-=(const vec4& rhs)
		{
			*this = *this - rhs;
			return *this;
		}

		vec4 vec4::operator-(const vec4& rhs) const
		{
			using namespace DirectX;
			XMVECTOR lhsVec = XMLoadFloat4(&float4);
			XMVECTOR rhsVec = XMLoadFloat4(&rhs.float4);
			XMVECTOR resultVec = XMVectorSubtract(lhsVec, rhsVec);
			vec4 result;
			XMStoreFloat4(&result.float4, resultVec);
			return result;
		}

		vec4 vec4::operator-() const
		{
			return vec4(-x, -y, -z, -w);
		}

		const float& vec4::operator[](int index) const
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			default: throw std::out_of_range("Index out of range");
			}
		}

		float& vec4::operator[](int index)
		{
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			default: throw std::out_of_range("Index out of range");
			}
		}

		float vec4::length() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat4(&float4);
			return XMVectorGetX(XMVector4Length(vec));
		}

		vec4 vec4::normalize() const
		{
			using namespace DirectX;
			XMVECTOR vec = XMLoadFloat4(&float4);
			XMVECTOR resultVec = XMVector4Normalize(vec);
			vec4 result;
			XMStoreFloat4(&result.float4, resultVec);
			return result;
		}

		mat3::mat3(const DirectX::XMMATRIX& mat)
		{
			DirectX::XMStoreFloat3x3(&data, mat);
		}

		mat3::mat3()
		{
			DirectX::XMStoreFloat3x3(&data, DirectX::XMMatrixIdentity());
		}

		mat3::mat3(const vec3 row0, const vec3 row1, const vec3 row2, const vec3 row3)
		{
			rowData[0] = row0;
			rowData[1] = row1;
			rowData[2] = row2;
		}

		vec3 mat3::operator*(const vec3& rhs) const
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&rhs.float3);
			DirectX::XMVECTOR resultVec = DirectX::XMVector3Transform(vec, mat);
			vec3 result;
			DirectX::XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		mat3 mat3::operator*(const mat3& rhs) const
		{
			DirectX::XMMATRIX lhsMat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMMATRIX rhsMat = DirectX::XMLoadFloat3x3(&rhs.data);
			DirectX::XMMATRIX resultMat = DirectX::XMMatrixMultiply(lhsMat, rhsMat);
			mat3 result(resultMat);
			return result;
		}

		vec3& mat3::operator[](int index)
		{
			return rowData[index];
		}

		const vec3& mat3::operator[](int index) const
		{
			return rowData[index];
		}

		mat3 mat3::identity()
		{
			return mat3();
		}

		void mat3::inverse()
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMMATRIX invMat = DirectX::XMMatrixInverse(nullptr, mat);
			DirectX::XMStoreFloat3x3(&data, invMat);
		}

		void mat3::translate(const vec3& translation)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(transMat, mat);
			DirectX::XMStoreFloat3x3(&data, resultMat);
		}

		void mat3::rotate(float angle, const vec3& axis)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMVECTOR axisVec = DirectX::XMLoadFloat3(&axis.float3);
			DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(axisVec, angle);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(rotMat, mat);
			DirectX::XMStoreFloat3x3(&data, resultMat);
		}

		void mat3::scale(const vec3& scale)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(scaleMat, mat);
			DirectX::XMStoreFloat3x3(&data, resultMat);
		}

		void mat3::transpose()
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat3x3(&data);
			DirectX::XMMATRIX transposedMat = DirectX::XMMatrixTranspose(mat);
			DirectX::XMStoreFloat3x3(&data, transposedMat);
		}

		mat4::mat4(const DirectX::XMMATRIX& mat)
		{
			DirectX::XMStoreFloat4x4(&data, mat);
		}

		mat4::mat4()
		{
			DirectX::XMStoreFloat4x4(&data, DirectX::XMMatrixIdentity());
		}

		mat4::mat4(const vec4 row0, const vec4 row1, const vec4 row2, const vec4 row3)
		{
			rowData[0] = row0;
			rowData[1] = row1;
			rowData[2] = row2;
			rowData[3] = row3;
		}

		vec3 mat4::operator*(const vec3& rhs) const
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&rhs.float3);
			DirectX::XMVECTOR resultVec = DirectX::XMVector3Transform(vec, mat);
			vec3 result;
			DirectX::XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		mat4 mat4::operator*(const mat4& rhs) const
		{
			DirectX::XMMATRIX lhsMat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMMATRIX rhsMat = DirectX::XMLoadFloat4x4(&rhs.data);
			DirectX::XMMATRIX resultMat = DirectX::XMMatrixMultiply(lhsMat, rhsMat);
			mat4 result(resultMat);
			return result;
		}

		vec4& mat4::operator[](int index)
		{
			return rowData[index];
		}

		const vec4& mat4::operator[](int index) const
		{
			return rowData[index];
		}

		mat4 mat4::identity()
		{
			return mat4();
		}

		void mat4::inverse()
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMMATRIX invMat = DirectX::XMMatrixInverse(nullptr, mat);
			DirectX::XMStoreFloat4x4(&data, invMat);
		}

		void mat4::translate(const vec3& translation)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(transMat, mat);
			DirectX::XMStoreFloat4x4(&data, resultMat);
		}

		void mat4::rotate(float angle, const vec3& axis)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMVECTOR axisVec = DirectX::XMLoadFloat3(&axis.float3);
			DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(axisVec, angle);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(rotMat, mat);
			DirectX::XMStoreFloat4x4(&data, resultMat);
		}

		void mat4::scale(const vec3& scale)
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			DirectX::XMMATRIX resultMat = XMMatrixMultiply(scaleMat, mat);
			DirectX::XMStoreFloat4x4(&data, resultMat);
		}

		void mat4::transpose()
		{
			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&data);
			DirectX::XMMATRIX transposedMat = DirectX::XMMatrixTranspose(mat);
			DirectX::XMStoreFloat4x4(&data, transposedMat);
		}

		vec4 normalize(const vec4& vector)
		{
			return vector.normalize();
		}

		vec3 normalize(const vec3& vector)
		{
			return vector.normalize();
		}

		vec2 normalize(const vec2& vector)
		{
			return vector.normalize();
		}

		mat4 inverse(const mat4& matrix)
		{
			mat4 newMatrix{ matrix };
			newMatrix.inverse();
			return newMatrix;
		}

		mat3 inverse(const mat3& matrix)
		{
			mat3 newMatrix{ matrix };
			newMatrix.inverse();
			return newMatrix;
		}

		mat4 transpose(const mat4& matrix)
		{
			mat4 newMatrix{ matrix };
			newMatrix.transpose();
			return newMatrix;
		}

		mat3 transpose(const mat3& matrix)
		{
			mat3 newMatrix{ matrix };
			newMatrix.transpose();
			return newMatrix;
		}

		vec3 cross(const vec3& lhs, const vec3& rhs)
		{
			DirectX::XMVECTOR lhsVec = DirectX::XMLoadFloat3(&lhs.float3);
			DirectX::XMVECTOR rhsVec = DirectX::XMLoadFloat3(&rhs.float3);
			DirectX::XMVECTOR resultVec = DirectX::XMVector3Cross(lhsVec, rhsVec);
			vec3 result;
			DirectX::XMStoreFloat3(&result.float3, resultVec);
			return result;
		}

		float dot(const vec2& lhs, const vec2& rhs)
		{
			DirectX::XMVECTOR lhsVec = DirectX::XMLoadFloat2(&lhs.float2);
			DirectX::XMVECTOR rhsVec = DirectX::XMLoadFloat2(&rhs.float2);
			DirectX::XMVECTOR resultVec = DirectX::XMVector2Dot(lhsVec, rhsVec);
			return DirectX::XMVectorGetX(resultVec);
		}

		float dot(const vec3& lhs, const vec3& rhs)
		{
			DirectX::XMVECTOR lhsVec = DirectX::XMLoadFloat3(&lhs.float3);
			DirectX::XMVECTOR rhsVec = DirectX::XMLoadFloat3(&rhs.float3);
			DirectX::XMVECTOR resultVec = DirectX::XMVector3Dot(lhsVec, rhsVec);
			return DirectX::XMVectorGetX(resultVec);
		}

		float dot(const vec4& lhs, const vec4& rhs)
		{
			DirectX::XMVECTOR lhsVec = DirectX::XMLoadFloat4(&lhs.float4);
			DirectX::XMVECTOR rhsVec = DirectX::XMLoadFloat4(&rhs.float4);
			DirectX::XMVECTOR resultVec = DirectX::XMVector4Dot(lhsVec, rhsVec);
			return DirectX::XMVectorGetX(resultVec);
		}

		mat4 rotate(const mat4& matrix, float angle, vec3 axis)
		{
			mat4 newMatrix{ matrix };
			newMatrix.rotate(angle, axis);
			return newMatrix;
		}

		mat3 rotate(const mat3& matrix, float angle, vec3 axis)
		{
			mat3 newMatrix{ matrix };
			newMatrix.rotate(angle, axis);
			return newMatrix;
		}

		mat4 scale(const mat4& matrix, vec3 scaleVector)
		{
			mat4 newMatrix{ matrix };
			newMatrix.scale(scaleVector);
			return newMatrix;
		}

		mat3 scale(const mat3& matrix, vec3 scaleVector)
		{
			mat3 newMatrix{ matrix };
			newMatrix.scale(scaleVector);
			return newMatrix;
		}

		mat4 translate(const mat4& matrix, vec3 translateVector)
		{
			mat4 newMatrix{ matrix };
			newMatrix.translate(translateVector);
			return newMatrix;
		}

		mat3 translate(const mat3& matrix, vec3 translateVector)
		{
			mat3 newMatrix{ matrix };
			newMatrix.translate(translateVector);
			return newMatrix;
		}

		mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up)
		{
			DirectX::XMVECTOR eyeVec = DirectX::XMLoadFloat3(&eye.float3);
			DirectX::XMVECTOR targetVec = DirectX::XMLoadFloat3(&target.float3);
			DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up.float3);
			DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookAtLH(eyeVec, targetVec, upVec);
			return mat4(lookAtMat);
		}

		mat4 perspective(float fovY, float aspect, float nearZ, float farZ)
		{
			DirectX::XMMATRIX perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
			return mat4(perspectiveMat);
		}

		mat4 orthographic(float width, float height, float nearZ, float farZ)
		{
			DirectX::XMMATRIX orthographicMat = DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ);
			return mat4(orthographicMat);
		}

		float degrees(float radians)
		{
			return radians * (180.0f / DirectX::XM_PI);
		}

		float radians(float degrees)
		{
			return degrees * (DirectX::XM_PI / 180.0f);
		}

	}
}