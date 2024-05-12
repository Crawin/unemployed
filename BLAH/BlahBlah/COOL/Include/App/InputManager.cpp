#include "framework.h"
#include "InputManager.h"
#include "Network/Client.h"

void InputManager::HandleMouseInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//WM_LBUTTONDOWN
	//WM_RBUTTONDOWN
	//WM_LBUTTONUP
	//WM_RBUTTONUP
	//WM_MOUSEMOVE
	m_LButtonReleased = false;
	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (m_MouseCapture) SetCapture(hWnd);
		GetCursorPos(&m_CurMouse);
		GetCursorPos(&m_BefMouse);

		m_LButtonState = true;
		m_Dragging = true;
		
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		ReleaseCapture();
		GetCursorPos(&m_CurMouse);
		m_MouseDif = { 0,0 };

		m_LButtonReleased = true;
		m_LButtonState = false;
		m_Dragging = false;

		break;
	case WM_MOUSEMOVE:
	{
		GetCursorPos(&m_CurMouse);
		if (m_Dragging && m_MouseCapture) {
			m_MouseDif.x = m_CurMouse.x - m_BefMouse.x;
			m_MouseDif.y = m_CurMouse.y - m_BefMouse.y;
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
		break;
	case 'O':
		Client::GetInstance().Send_Room(pMAKEROOM, NULL);
		break;
	case 'I':
		//std::cout << "입장할 방 번호를 입력하세요: ";
		//std::string s_gameNum;
		//std::cin >> s_gameNum;
		//int i_gameNum = atoi(s_gameNum.c_str());
		//Client::GetInstance().Send_Room(pENTERROOM, i_gameNum);
		Client::GetInstance().Send_Room(pENTERROOM, 10000);
		break;
	}
}
