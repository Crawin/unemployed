#pragma once

#include "Component.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI 관련 컴포넌트들
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using ButtonEventFunction = std::function<void(Entity*)>;

namespace component {
	/////////////////////////////////////////////////////////
	// UICanvas Component
	// UI 캔버스, 해당 ui가 켜지면 다른 것들도 사용 됨
	//
	class UICanvas : public ComponentBase<UICanvas> {
		int m_CanvasID = -1;
		bool m_On = false;

		static int m_NextID;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void ShowUI() { m_On = true; }
		void HideUI() { m_On = false; }

		bool IsActive() const { return m_On; }
	};

	/////////////////////////////////////////////////////////
	// UITransform Component
	// 스크린공간상 중간 위치와 사이즈
	//
	class UITransform : public ComponentBase<UITransform> {
		SIZE m_Center = { 1280 / 2, 720 / 2 };
		SIZE m_Size = { 200, 200 };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		const SIZE& GetCenter() const { return m_Center; }
		const SIZE& GetSize() const { return m_Size; }
	};

	/////////////////////////////////////////////////////////
	// UIRenderer Component
	// UI 렌더러, 
	//
	class UIRenderer : public ComponentBase<UIRenderer> {
		int m_MaterialID = -1;

		bool m_IsSprite = false;
		SIZE m_SpriteSize = { 0,0 };
		int m_SpriteNum = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		int GetMaterial() const { return m_MaterialID; }

		void SetMaterial(int idx) { m_MaterialID = idx; }

		const SIZE& GetSpriteSize() const { return m_SpriteSize; }

		// root constant로 바로 보내기 위해 레퍼런스를 붙인다.
		const int& GetCurSprite() const { return m_SpriteNum; }
	};

	/////////////////////////////////////////////////////////
	// Button Component
	// 버튼이 down, released 되면 실행되는 함수를 가짐
	//
	class Button : public ComponentBase<Button> {
		ButtonEventFunction m_OnButtonClicked = nullptr;
		bool m_Pressing = false;
		bool m_On = false;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};


}
