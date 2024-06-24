#pragma once

namespace WaveE
{
	class WCamera
	{
	public:
		WCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane);

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix() const;
		glm::vec3 GetPosition() const;

		void MoveForward(float delta);
		void MoveRight(float delta);
		void MoveUp(float delta);
		void Rotate(float yaw, float pitch);
		void SetAspectRatio(float aspectRatio);

		void SetPosition(glm::vec3 pos);
		void SetLootAt(glm::vec3 lookAt);

	private:
		void UpdateCameraVectors();

		glm::vec3 m_position;
		glm::vec3 m_front;
		glm::vec3 m_up;
		glm::vec3 m_right;
		glm::vec3 m_worldUp;

		float m_yaw;
		float m_pitch;
		float m_fov;
		float m_aspectRatio;
		float m_nearPlane;
		float m_farPlane;
	};
}

