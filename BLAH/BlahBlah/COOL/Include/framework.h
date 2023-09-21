// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <string>
#include <wrl.h>			// COM
#include <shellapi.h>

#include <vector>
#include <map>


// DirectX 12 Headers
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <Mmsystem.h>

// debug only
#include "OnDebug.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")


enum MY_ROOTSIGN_PARAM {
	CAMERA_DATA = 0,						// cbv (CamPos, farplane, v Mat, p Mat, vp Mat)
	LIGHTS = 1,								// cbv (struct LIGHT)
	WORLD_MATIRX = 2,						// root constant float4x4 (world matrix)
	WORLD_MATIRX_INSTANCED = 3,				// cbv (array of world matrix)
	SHADER_BASE_PROPERTY = 4,				// root constant float4x4(shader base property)
	BINDLESS_RESOURCE = 5,					// descriptor heap (all shader resource(srv))

	PARAM_MAX
};
