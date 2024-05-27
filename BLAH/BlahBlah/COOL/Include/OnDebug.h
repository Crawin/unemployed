#pragma once
// 디버그 모드 활성화 될 것들 모음집.txt


// 디버그 모드에 테스트용
#ifdef _DEBUG
#include <iostream>
#include <dxgidebug.h>

// 콘솔창 활셩화
#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
#endif


// 스트링을 출력한다
inline void DebugPrint(const std::string& str)
{
#ifdef _DEBUG
	std::cout << str << "\n";
#endif
}

inline void DebugPrintVector(const XMFLOAT3& v, std::string str = std::string())
{
#ifdef _DEBUG
	std::cout << str << std::format(": {}, {}, {}", v.x, v.y, v.z) << "\n";
#endif
}


// 
#define CHECK_CREATE_FAILED(result, msg) if (!result) { DebugPrint(std::format("Failed! {}({}) {}", __FUNCTION__, __LINE__, msg)); return false; }
#define PRINT_ERROR(msg) DebugPrint(std::format("{}: {}", __LINE__, msg))
#define ERROR_QUIT(msg) { DebugPrint(msg); DebugPrint("\nExiting program.."); system("pause"); exit(-1); }
