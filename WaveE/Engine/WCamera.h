#pragma once

namespace WaveE
{
	class WCamera
	{
	public:
		WCamera(wma::vec3 position, float yaw, float pitch, float fov, float aspectRatio, float nearPlane, float farPlane);

		wma::mat4 GetViewMatrix() const;
		wma::mat4 GetProjectionMatrix() const;
		wma::vec3 GetPosition() const;
		wma::vec3 GetForwards() const;
		wma::vec3 GetRight() const;
		wma::vec3 GetUp() const;
		wma::mat4 GetRotation() const;

		void MoveForward(float delta);
		void MoveRight(float delta);
		void MoveUp(float delta);
		void Rotate(float yaw, float pitch);
		void SetAspectRatio(float aspectRatio);

		void SetPosition(wma::vec3 pos);
		void SetLootAt(wma::vec3 lookAt);

	private:
		void UpdateCameraVectors();

		wma::vec3 m_position;
		wma::vec3 m_forwards;
		wma::vec3 m_up;
		wma::vec3 m_right;

		float m_yaw;
		float m_pitch;
		float m_fov;
		float m_aspectRatio;
		float m_nearPlane;
		float m_farPlane;
	};
}

