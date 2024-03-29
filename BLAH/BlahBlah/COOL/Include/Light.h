#pragma once

// light type
// 0 : Directional Light
// 1 : Spot Light
// 2 : Point Light

enum class LIGHT_TYPES {
	DIRECTIONAL_LIGHT = 0,		// 1 camera (ortho
	SPOT_LIGHT,					// 1 camera (perspective)
	POINT_LIGHT					// 6 cameras (perspective), render to texcube
};

// data to send GPU
struct LightData {
	XMFLOAT4 m_LightColor;

	XMFLOAT3 m_Direction;			// Transform에 연계하여 바뀐다.
	float m_Falloff;
	
	XMFLOAT3 m_Position;			// Transform에 연계하여 바뀐다.
	int m_LightType = 0;
	
	// for shadow map
	int m_ShadowMapResults[3] = {-1,};
	BOOL m_CascadedShadow = FALSE;	// main light 일 경우에만 켜진다.
	
	BOOL m_Active = TRUE;			// HLSL에서 bool은 4bytes, C++에서의 bool <- 1byte, BOOL은 int를 확장
	BOOL m_CastShadow = FALSE;		// 상황에 맞게 판단하여 그때그때 켜진다.
	int m_CameraIdx = -1;
	float m_Temperature = -1;		// 1 이상이라면 켜진거임		2000 ~ 7000 (주황 - 하양), 
};


struct CameraDataShader {
	XMFLOAT4X4 m_ViewMatrix;
	XMFLOAT4X4 m_ProjMatrix;
	XMFLOAT3 m_CameraPosition;
	float m_Padding;
};