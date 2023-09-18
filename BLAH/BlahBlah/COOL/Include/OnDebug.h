#pragma once
// ����� ��� Ȱ��ȭ �� �͵� ������.txt


// ����� ��忡 �׽�Ʈ��
#ifdef _DEBUG
#include <iostream>
#include <format>
#include <dxgidebug.h>

// �ܼ�â Ȱ��ȭ
#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
#endif

#include <format>

// ��Ʈ���� ����Ѵ�
inline void DebugPrint(const std::string& str)
{
#ifdef _DEBUG
	std::cout << str << std::endl;
#endif
}


// 
#define CHECK_CREATE_FAILED(result, msg) if (!result) { DebugPrint(std::format("Failed! {}({}) {}", __FUNCTION__, __LINE__, msg)); return false; }
#define PRINT_ERROR(msg) DebugPrint(std::format("{}: {}", __LINE__, msg))
