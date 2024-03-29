﻿#include "framework.h"
#include "InputManager.h"
#include "Network/Client.h"

void InputManager::HandleMouseInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//WM_LBUTTONDOWN
	//WM_RBUTTONDOWN
	//WM_LBUTTONUP
	//WM_RBUTTONUP
	//WM_MOUSEMOVE

	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		GetCursorPos(&m_BefMouse);

		m_Dragging = true;
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		ReleaseCapture();
		m_MouseDif = { 0,0 };
		m_Dragging = false;
		break;
	case WM_MOUSEMOVE:
	{
		if (m_Dragging) {
			POINT curPos;
			GetCursorPos(&curPos);
			m_MouseDif.x = curPos.x - m_BefMouse.x;
			m_MouseDif.y = curPos.y - m_BefMouse.y;
			SetCursorPos(m_BefMouse.x, m_BefMouse.y);
		}
		//m_BefMouse = curPos;
		
		break;
	}
	}
}

void InputManager::HandleKeyboardInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case VK_F8:
		m_DebugMode = !m_DebugMode;
		break;
	case 'P':
		Client::GetInstance().Connect_Server();
		Client::GetInstance().Send_Str("방생성");
		break;
	}
}
