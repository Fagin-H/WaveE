#include "stdafx.h"
#include "WCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace WaveE
{
	WCamera::WCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		UpdateCameraVectors();
	}

	glm::mat4 WCamera::GetViewMatrix() const
	{
		glm::vec3 w = glm::normalize(m_front);
		glm::vec3 u = glm::normalize(glm::cross(m_up, w));
		glm::vec3 v = glm::cross(w, u);

		glm::mat4 viewMat{
			glm::vec4{u.x, v.x, w.x, 0},
			glm::vec4{u.y, v.y, w.y, 0},
			glm::vec4{u.z, v.z, w.z, 0},
			glm::vec4{glm::dot(-m_position, u), glm::dot(-m_position, v), glm::dot(-m_position, w), 1}
		};

		return glm::transpose(viewMat);
	}

	glm::mat4 WCamera::GetProjectionMatrix() const
	{
		float a = 1.f / glm::tan(m_fov * 0.5f);
		float b = m_farPlane / (m_farPlane - m_nearPlane);

		glm::mat4 projMat{
			glm::vec4{a / m_aspectRatio,0,0,0},
			glm::vec4{0,a,0,0},
			glm::vec4{0,0,b,1},
			glm::vec4{0,0,-m_nearPlane * b,0}
		};

		return glm::transpose(projMat);
	}

	glm::vec3 WCamera::GetPosition() const
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

	void WCamera::SetPosition(glm::vec3 pos)
	{
		m_position = pos;
	}

	void WCamera::SetLootAt(glm::vec3 lookAt)
	{
		glm::vec3 direction = glm::normalize(lookAt - m_position);
		m_pitch = glm::degrees(asin(direction.y));
		m_yaw = glm::degrees(atan2(direction.z, direction.x)) - 90.0f;

		UpdateCameraVectors();
	}

	void WCamera::UpdateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}
}