#pragma once

// light type
// 0 : Directional Light
// 1 : Spot Light
// 2 : Point Light

// data to send GPU
struct LightData {
	XMFLOAT4 m_LightColor;
	
	XMFLOAT3 m_Direction;
	float m_Falloff;
	
	XMFLOAT3 m_Position;
	int m_LightType = 0;
	
	// for shadow map
	XMUINT3 m_ShadowMapResults = { (uint32_t)-1, (uint32_t)-1, (uint32_t)-1 };
	BOOL m_CascadedShadow = FALSE;
	
	BOOL m_Active = TRUE;			// HLSL에서 bool은 4bytes, C++에서의 bool <- 1byte, BOOL은 int를 확장
	BOOL m_CastShadow = FALSE;
};
