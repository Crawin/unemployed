// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.


#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN					// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#include <windows.h>
#include <hidusage.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <string>
#include <format>

#include <wrl.h>			// COM
#include <shellapi.h>

#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <functional>
#include <bitset>
#include <any>

// DirectX 12 Headers
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
//#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "DDSTextureLoader12.h"

#include <Mmsystem.h>

#include "DXMath.h"

// debug only
#include "OnDebug.h"

//using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "jsoncpp.lib")

#include "Enums.h"

// component count
#define COMPONENT_COUNT 15


std::string ExtractFileName(const std::string& fullPath);


