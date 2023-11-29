#include "framework.h"
#include "Application.h"
#include "Renderer/Renderer.h"

Application::Application()
{
}

Application::~Application()
{
	// �ױ� ���� ����ִ� �ֵ� Ȯ���ϰ� ����
#if defined(_DEBUG)
	IDXGIDebug1* debug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&debug);
	HRESULT hResult = debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	debug->Release();
#endif
}

bool Application::InitWindow()
{
	// ������ ����
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

bool Application::Init(HINSTANCE hInst, const SIZE& wndSize)
{
	m_hInst = hInst;
	m_windowSize = wndSize;

	// windows ���� �� â ���� �Ѵ�
	CHECK_CREATE_FAILED(InitWindow(), "������ ���� ����");

	// �������� �����
	CHECK_CREATE_FAILED(Renderer::GetInstance().Init(m_windowSize, m_hWnd), "������ ���� ����");

	// high input mouse
	// ����
	// https://learn.microsoft.com/ko-kr/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = m_hWnd;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	// ���Ŵ����� ���� �� �ε�?

	return true;
}

int Application::StartProgram()
{
	MSG Message;

	while (m_GameLoop) {
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else {
			// ���� ����

			Renderer::GetInstance().Render();
			/*
#ifdef _DEBUG
			TCHAR szTitle[30];
			swprintf(szTitle, L"FPS : %.1f", 1 / 1);
			SetConsoleTitle(szTitle);
#endif // DEBUG
*/
		}
	}
	return 0;
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// �ӽ�
	static bool dragging = false;

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		Application::GetInstance().m_GameLoop = false;
		break;
	case WM_INPUT:
	{
		UINT dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			int xPosRelative = raw->data.mouse.lLastX;
			int yPosRelative = raw->data.mouse.lLastY;

			// �ӽ�
			// �� ����� ���⼭ �ҵ�?
			if (dragging) Renderer::GetInstance().MouseInput(xPosRelative, yPosRelative);

			//DebugPrint(std::format("x: {}, y: {}", xPosRelative, yPosRelative));
		}
		break;
	}
	case WM_LBUTTONDOWN:
		dragging = true;
		break;
	case WM_LBUTTONUP:
		dragging = false;
		break;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
