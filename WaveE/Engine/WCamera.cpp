#include "stdafx.h"
#include "WCamera.h"

namespace WaveE
{
	WCamera::WCamera(wma::vec3 position, wma::vec3 up, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		UpdateCameraVectors();
	}

	wma::mat4 WCamera::GetViewMatrix() const
	{
		wma::mat4 viewMat = wma::lookAt(m_position, m_position + m_front, m_up);

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

	void WCamera::MoveForward(float delta)
	{
		m_position += m_front * delta;
	}

	void WCamera::MoveRight(float delta)
	{
		m_position += m_right * delta;
	}

	void WCamera::MoveUp(float delta)
	{
		m_position += m_worldUp * delta;
	}

	void WCamera::Rotate(float yaw, float pitch)
	{
		m_yaw += yaw;
		m_pitch += pitch;

		// Constrain the pitch
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

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
		wma::vec3 front;
		front.x = cos(wma::radians(m_yaw)) * cos(wma::radians(m_pitch));
		front.y = sin(wma::radians(m_pitch));
		front.z = sin(wma::radians(m_yaw)) * cos(wma::radians(m_pitch));
		m_front = wma::normalize(front);
		// Also re-calculate the Right and Up vector
		m_right = wma::normalize(wma::cross(m_front, m_worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_up = wma::normalize(wma::cross(m_right, m_front));
	}
}