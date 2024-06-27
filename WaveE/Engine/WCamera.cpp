#include "stdafx.h"
#include "WCamera.h"

namespace WaveE
{
	WCamera::WCamera(wma::vec3 position, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_position(position), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		UpdateCameraVectors();
	}

	wma::mat4 WCamera::GetViewMatrix() const
	{
		wma::mat4 viewMat = wma::lookAt(m_position, m_position + m_forwards, m_up);

		return viewMat;
	}

	wma::mat4 WCamera::GetProjectionMatrix() const
	{
		wma::mat4 projMat = wma::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

		return projMat;
	}

	wma::vec3 WCamera::GetPosition() const
	{
		return m_position;
	}

	wma::vec3 WCamera::GetForwards() const
	{
		return m_forwards;
	}

	wma::vec3 WCamera::GetRight() const
	{
		return m_right;
	}

	wma::vec3 WCamera::GetUp() const
	{
		return m_up;
	}

	wma::mat4 WCamera::GetRotation() const
	{
		return wma::rotation(wma::radians(m_pitch), wma::radians(m_yaw));
	}

	void WCamera::MoveForward(float delta)
	{
		m_position += m_forwards * delta;
	}

	void WCamera::MoveRight(float delta)
	{
		m_position += m_right * delta;
	}

	void WCamera::MoveUp(float delta)
	{
		m_position += m_up * delta;
	}

	void WCamera::Rotate(float yaw, float pitch)
	{
		m_yaw += yaw;
		m_pitch += pitch;

		// Constrain the pitch
		if (m_pitch > 89.0f)
		{
			m_pitch = 89.0f;
		}
		if (m_pitch < -89.0f)
		{
			m_pitch = -89.0f;
		}

		UpdateCameraVectors();
	}

	void WCamera::SetAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
	}

	void WCamera::SetPosition(wma::vec3 pos)
	{
		m_position = pos;
	}

	void WCamera::SetLootAt(wma::vec3 lookAt)
	{
		wma::vec3 direction = wma::normalize(lookAt - m_position);
		m_pitch = wma::degrees(asin(direction.y));
		m_yaw = wma::degrees(atan2(direction.z, direction.x)) - 90.0f;

		UpdateCameraVectors();
	}

	void WCamera::UpdateCameraVectors()
	{
		// Calculate the new Front vector

		float cosYaw = cos(wma::radians(m_yaw));
		float sinYaw = sin(wma::radians(m_yaw));
		float cosPitch = cos(wma::radians(m_pitch));
		float sinPitch = sin(wma::radians(m_pitch));

		m_forwards = wma::vec3{ sinYaw * cosPitch, -sinPitch, cosYaw * cosPitch };
		m_right = wma::vec3{ cosYaw, 0.f, -sinYaw };
		m_up = wma::vec3{ sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	}
}