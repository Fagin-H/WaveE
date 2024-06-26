#pragma once

namespace WaveE
{
	enum MouseButtonKey
	{
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE
	};

	class WInput
	{
		WAVEE_SINGLETON(WInput)
	public:
		void EndFrameUpdate();

		bool IsKeyDown(UINT key) const;
		bool WasKeyPressed(UINT key) const;
		bool WasKeyReleased(UINT key) const;

		bool IsMouseButtonDown(MouseButtonKey button) const;
		bool WasMouseButtonPressed(MouseButtonKey button) const;
		bool WasMouseButtonReleased(MouseButtonKey button) const;

		wma::vec2 GetMousePosition() const;
		wma::vec2 GetMouseDelta() const;

	private:
		WInput() = default;
		
		wma::vec2 m_mousePos;
	};
}

