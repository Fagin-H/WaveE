#include "stdafx.h"
#include "WInput.h"
#include "WaveManager.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WInput)

	void WInput::EndFrameUpdate()
	{
		WaveManager::MouseState currentMouseState = WaveManager::Instance()->GetMouseState(false);
		WaveManager::MouseState previousMouseState = WaveManager::Instance()->GetMouseState(true);

		m_mousePos.x = currentMouseState.mousePosition.x;
		m_mousePos.y = currentMouseState.mousePosition.y;
	}

	bool WInput::IsKeyDown(UINT key) const
	{
		return WaveManager::Instance()->GetKeyState(false).test(key);
	}

	bool WInput::WasKeyPressed(UINT key) const
	{
		return WaveManager::Instance()->GetKeyState(false).test(key) && !WaveManager::Instance()->GetKeyState(true).test(key);
	}

	bool WInput::WasKeyReleased(UINT key) const
	{
		return WaveManager::Instance()->GetKeyState(true).test(key) && !WaveManager::Instance()->GetKeyState(false).test(key);
	}

	bool WInput::IsMouseButtonDown(MouseButtonKey button) const
	{
		switch (button)
		{
		case WaveE::MOUSE_LEFT:
			return WaveManager::Instance()->GetMouseState(false).leftMouseButton;
			break;
		case WaveE::MOUSE_RIGHT:
			return WaveManager::Instance()->GetMouseState(false).rightMouseButton;
			break;
		case WaveE::MOUSE_MIDDLE:
			return WaveManager::Instance()->GetMouseState(false).middleMouseButton;
			break;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Invalid mouse key!");
			break;
		}
		return false;
	}

	bool WInput::WasMouseButtonPressed(MouseButtonKey button) const
	{
		switch (button)
		{
		case WaveE::MOUSE_LEFT:
			return WaveManager::Instance()->GetMouseState(false).leftMouseButton && !WaveManager::Instance()->GetMouseState(true).leftMouseButton;
			break;
		case WaveE::MOUSE_RIGHT:
			return WaveManager::Instance()->GetMouseState(false).rightMouseButton && !WaveManager::Instance()->GetMouseState(true).rightMouseButton;
			break;
		case WaveE::MOUSE_MIDDLE:
			return WaveManager::Instance()->GetMouseState(false).middleMouseButton && !WaveManager::Instance()->GetMouseState(true).middleMouseButton;
			break;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Invalid mouse key!");
			break;
		}
		return false;
	}

	bool WInput::WasMouseButtonReleased(MouseButtonKey button) const
	{
		switch (button)
		{
		case WaveE::MOUSE_LEFT:
			return WaveManager::Instance()->GetMouseState(true).leftMouseButton && !WaveManager::Instance()->GetMouseState(false).leftMouseButton;
			break;
		case WaveE::MOUSE_RIGHT:
			return WaveManager::Instance()->GetMouseState(true).rightMouseButton && !WaveManager::Instance()->GetMouseState(false).rightMouseButton;
			break;
		case WaveE::MOUSE_MIDDLE:
			return WaveManager::Instance()->GetMouseState(true).middleMouseButton && !WaveManager::Instance()->GetMouseState(false).middleMouseButton;
			break;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Invalid mouse key!");
			break;
		}
		return false;
	}

	wma::vec2 WInput::GetMousePosition() const
	{
		return m_mousePos;
	}

	wma::vec2 WInput::GetMouseDelta() const
	{
		return WaveManager::Instance()->GetMouseState(false).delta;
	}
}