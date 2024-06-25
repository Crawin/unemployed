#pragma once

// 윈도우를 초기화 하고 게임의 메인루프가 있는 곳
// 별로 볼 일 없을 것이다...

class SceneManager;
class Timer;

class Application
{
public:
	static Application& GetInstance() {
		static Application inst;
		return inst;
	}

private:
	Application();
	~Application();

	bool InitWindow();

	void Tick();

public:
	SIZE GetSize() const { return m_windowSize; }
	HWND GethWnd() const { return m_hWnd; }
	bool GetGameLoop() const { return m_GameLoop; }
	void SetGameLoopFalse() { m_GameLoop = false; }

	void GetWindowCenterPos(POINT& pos);

	bool Init(HINSTANCE hInst, const SIZE& wndSize);
	int StartProgram();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	//FMOD_SYSTEM* getFmodSystem() { return m_pSoundSystem; }

private:
	HINSTANCE	m_hInst = 0;
	HWND		m_hWnd = 0;
	HDC			m_hdc = 0;
	SIZE		m_windowSize = {};

	Timer* m_Timer = nullptr;
	//LARGE_INTEGER m_Sec;
	//LARGE_INTEGER m_Time;
	//float m_DeltaTime = 0;
	//float m_TimeCnt = 0;

	bool m_ShowGrid = false;
	bool m_GameLoop = true;

	//FMOD_SYSTEM* m_pSoundSystem;
	SceneManager* m_SceneManager = nullptr;

};