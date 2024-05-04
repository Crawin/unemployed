#pragma once

#include "Component.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI 관련 컴포넌트들
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace component {
	/////////////////////////////////////////////////////////
	// UICanvas Component
	// UI 캔버스, 해당 ui가 켜지면 다른 것들도 렌더 됨
	//
	class UICanvas : public ComponentBase<UICanvas> {
		int m_CanvasID = -1;
		bool m_On = false;
		SIZE m_Pivot = { 1280 / 2, 720 / 2 };
		SIZE m_Size = { 1280 / 2, 720 / 2 };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};
}
