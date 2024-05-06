﻿#pragma once

class InputManager
{
// 싱글톤 지옥
	InputManager() {}
	~InputManager() {}
	InputManager(const InputManager& other) = delete;
	InputManager& operator=(const InputManager& other) = delete;

public:

	static InputManager& GetInstance() {
		static InputManager inst;
		return inst;
	}

	void HandleMouseInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	void HandleKeyboardInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	const POINT& GetMouseMove() const { return m_MouseDif; }
	const POINT& GetMouseCurrentPosition() const { return m_CurMouse; }

	bool GetDrag() const { return m_Dragging; }

	bool GetDebugMode() const { return m_DebugMode; }

	bool IsMouseLeftDown() const { return m_LButtonState; }
private:
	// mosue input
	BYTE lpb[sizeof(RAWINPUT)] = { 0, };

	POINT m_BefMouse = { 0,0 };
	POINT m_CurMouse = { 0,0 };
	POINT m_MouseDif = { 0,0 };

	bool m_Dragging = false;
	bool m_MouseCapture = true;

	bool m_LButtonState = false;

	bool m_DebugMode = false;

};

