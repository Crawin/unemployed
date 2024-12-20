﻿#include "framework.h"
#include "InputManager.h"
#include "Application.h"
#include "Network/Client.h"
#include "FMODsound/FmodSound.h"

// todo 임시코드이다. 지우자
#include "Scene/SceneManager.h"
#include "Renderer/Renderer.h"

InputManager::InputManager()
{
	POINT center;
	Application::GetInstance().GetWindowCenterPos(center);

	SetCursorPos(center.x, center.y);

	SetUIState(false);
}

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
		//if (m_MouseCapture) SetCapture(hWnd);
		GetCursorPos(&m_CurMouse);
		//GetCursorPos(&m_WndCenter);

		m_LButtonState = true;
		m_Dragging = true;
		
		break;
	case WM_LBUTTONUP:
		//ReleaseCapture();
		GetCursorPos(&m_CurMouse);
		m_MouseDif = { 0,0 };

		m_LButtonReleased = true;
		m_LButtonState = false;
		m_Dragging = false;

		break;

	case WM_RBUTTONDOWN:
		m_LButtonState = true;
		break;

	case WM_RBUTTONUP:
		m_LButtonState = false;
		break;


	case WM_MOUSEMOVE:
	{
		GetCursorPos(&m_CurMouse);

		// main game state
		if (m_MouseCapture) 
		{
			m_MouseDif.x += m_CurMouse.x - m_WndCenter.x;
			m_MouseDif.y += m_CurMouse.y - m_WndCenter.y;
			SetCursorPos(m_WndCenter.x, m_WndCenter.y);
		}
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

	case VK_F7:
		Application::GetInstance().ChangeBorderlessMode(!Application::GetInstance().GetWindowed());
		//Renderer::GetInstance().ChangeSwapChainState(!Renderer::GetInstance().IsWindowed());
		break;

	case 'P':
		//Client::GetInstance().Connect_Server();
		break;
	case 'O':
		Application::GetInstance().GetSceneManager()->PossessPlayer(true);
		Client::GetInstance().Send_Room(pMAKEROOM, NULL);
		break;
	case 'I':
		//std::cout << "입장할 방 번호를 입력하세요: ";
		//std::string s_gameNum;
		//std::cin >> s_gameNum;
		//int i_gameNum = atoi(s_gameNum.c_str());
		//Client::GetInstance().Send_Room(pENTERROOM, i_gameNum);
		Application::GetInstance().GetSceneManager()->PossessPlayer(false);
		Client::GetInstance().Send_Room(pENTERROOM, 10000);
		break;
	case 'Q':
		FMOD_INFO::GetInstance().play_unloop_sound(DirectX::XMFLOAT3{0,0,0}, SOUND_TYPE::DUCK, "ducktest");
		break;
	case VK_ESCAPE:
		// test code
		SetUIState(m_MouseCapture);
		break;
		break;
	}
}

void InputManager::SetUIState(bool uiState)
{
	// set input state
	if (uiState == true) {
		m_UIState = true;

		m_MouseDif = { 0,0 };

		ReleaseCapture();
		m_MouseCapture = false;
		int result;
		//while (result = ShowCursor(true) < 0);
		SetCursor(GetCursor());
		//if (m_BefCursor)
		//	SetCursor(m_BefCursor);
		
		//do {
		//	result = ShowCursor(true);
		//} while (result < 0);
		//SetCursor(IDC_CURSOR);

		DebugPrint(std::format("ui true ret: {}", result));
		//DebugPrint(std::format("curs ret: {}", result));

		// todo
		// set keyboard state to disable
	}
	else {
		m_UIState = false;

		SetCapture(Application::GetInstance().GethWnd());
		m_MouseCapture = true;

		int result;
		//while (result = ShowCursor(false) >= 0);

		auto returnVal = SetCursor(NULL);

		if (m_BefCursor == NULL)
			m_BefCursor = returnVal;

		DebugPrint(std::format("ui false ret: {}", result));

		POINT pt;
		Application::GetInstance().GetWindowCenterPos(pt);

		m_WndCenter = pt;
		m_CurMouse = pt;
		SetCursorPos(pt.x, pt.y);

		//GetCursorPos(&m_CurMouse);
		//GetCursorPos(&m_BefMouse);

		// todo
		// set keyboard state to disable
	}
}
