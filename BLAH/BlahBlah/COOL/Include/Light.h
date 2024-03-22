#pragma once

// light type
// 0 : Directional Light
// 1 : Spot Light
// 2 : Point Light

// data to send GPU
struct LightData {
	XMFLOAT4 m_LightColor;
	
	XMFLOAT3 m_Direction;			// Transform에 연계하여 바뀐다.
	float m_Falloff;
	
	XMFLOAT3 m_Position;			// Transform에 연계하여 바뀐다.
	int m_LightType = 0;
	
	// for shadow map
	XMUINT3 m_ShadowMapResults = { (uint32_t)-1, (uint32_t)-1, (uint32_t)-1 };
	BOOL m_CascadedShadow = FALSE;	// main light 일 경우에만 켜진다.
	
	BOOL m_Active = TRUE;			// HLSL에서 bool은 4bytes, C++에서의 bool <- 1byte, BOOL은 int를 확장
	BOOL m_CastShadow = FALSE;		// 상황에 맞게 판단하여 그때그때 켜진다.
};
