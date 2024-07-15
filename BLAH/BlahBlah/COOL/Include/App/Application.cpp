#include "framework.h"
#include "Application.h"
#include "Renderer/Renderer.h"
#include "InputManager.h"
#include "Scene/SceneManager.h"
#include "Timer.h"
#include "FMODsound/FmodSound.h"
#include <thread>

#ifdef _DEBUG
#pragma warning(disable  : 4996)
#endif

Application::Application()
{
	m_Timer = new Timer;
}

Application::~Application()
{
	if (m_Timer) delete m_Timer;
	if (m_SceneManager) delete m_SceneManager;

	// 죽기 전에 살아있는 애들 확인하고 간다
#if defined(_DEBUG)
	IDXGIDebug1* debug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&debug);
	HRESULT hResult = debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	debug->Release();
#endif
}

bool Application::InitWindow()
{
	// 윈도우 생성
	WNDCLASSEX WndClass;

	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.lpfnWndProc = WndProc;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = m_hInst;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = L"Class Name";
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	RECT rt = { 0, 0, m_windowSize.cx, m_windowSize.cy };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, NULL);
	m_hWnd = CreateWindow(L"Class Name", L"Your Game Name Here", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		rt.right - rt.left, rt.bottom - rt.top, NULL, NULL, m_hInst, NULL);

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

void Application::Tick()
{
	// 게임 루프
	m_Timer->Update();
	m_SceneManager->Update(m_Timer->GetDeltaTime());
	Renderer::GetInstance().Render();


#ifdef _DEBUG
	TCHAR szTitle[30];
	swprintf(szTitle, L"FPS : %.1f", 1 / m_Timer->GetDeltaTime());
	SetConsoleTitle(szTitle);
#endif // DEBUG
}

void Application::GetWindowCenterPos(POINT& pos)
{
	RECT rt;
	GetWindowRect(Application::GetInstance().GethWnd(), &rt);

	pos = { (rt.right + rt.left) / 2, (rt.bottom + rt.top) / 2 };
}

bool Application::Init(HINSTANCE hInst, const SIZE& wndSize)
{
	m_hInst = hInst;
	m_windowSize = wndSize;

	// windows 생성 및 창 띄우기 한다
	CHECK_CREATE_FAILED(InitWindow(), "윈도우 생성 실패");

	// 렌더러를 만든다
	CHECK_CREATE_FAILED(Renderer::GetInstance().Init(m_windowSize, m_hWnd), "렌더러 생성 실패");

	// high input mouse
	// 참고
	// https://learn.microsoft.com/ko-kr/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = m_hWnd;
	CHECK_CREATE_FAILED(RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])), "RegisterRawInputDevices 실패");

	// 씬매니저의 생성 및 로드?
	auto commandList = Renderer::GetInstance().GetCommandList(0);
	m_SceneManager = new SceneManager();
	CHECK_CREATE_FAILED(m_SceneManager->Init(commandList, "Test"), "씬매니저 생성 실패");
	Renderer::GetInstance().SetSceneManager(m_SceneManager);
	Renderer::GetInstance().ExecuteAndEraseUploadHeap(commandList);

	return true;
}

int Application::StartProgram()
{
	std::thread fmod_thread(start_fmod, std::ref(FMOD_INFO::GetInstance()));
	MSG Message;
	m_Timer->Start();
	while (m_GameLoop) {
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else 
		{
			Tick();
		}
		SleepEx(0, true);
	}
	FMOD_INFO::GetInstance().set_client_on(false);
	fmod_thread.join();
	return 0;
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// 임시

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		Application::GetInstance().m_GameLoop = false;
		break;

	case WM_WINDOWPOSCHANGING:
		if (Application::GetInstance().m_SceneManager)
			Application::GetInstance().Tick();
		break;

	case WM_SYSKEYDOWN:
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		InputManager::GetInstance().HandleMouseInput(hWnd, msg, wParam, lParam);
		break;
	case WM_KEYDOWN:
		InputManager::GetInstance().HandleKeyboardInput(hWnd, msg, wParam, lParam);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
