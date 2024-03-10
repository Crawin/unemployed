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

	// for shadow map
	BOOL m_CastShadow = true;			// HLSL에서 bool은 4bytes, C++에서의 bool <- 1byte, BOOL은 int를 확장
	BOOL m_Active = true;
	int m_ShadowMapResult = -1;
	int m_LightType = 0;
};
