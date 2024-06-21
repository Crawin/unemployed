#pragma once

#include "Component.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 컨텐츠 관련 컴포넌트들, 게임 컨텐츠에 필요한 ui도 포함
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace component {
	/////////////////////////////////////////////////////////
	// Player Animation Controll component
	// Player를 쓰는 애를 위한 애니메이션 컨트롤 컴포넌트
	//
	class PlayerAnimControll : public ComponentBase<PlayerAnimControll> {
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float m_BefSpeed = 0;

		bool m_BefKeyDown = false;
	};

	/////////////////////////////////////////////////////////
	// Dia Animation Controll component
	// Dia를 쓰는 애를 위한 애니메이션 컨트롤 컴포넌트
	//
	class DiaAnimationControl : public ComponentBase<DiaAnimationControl> {
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float m_BefSpeed = 0;

		bool m_BefKeyDown = false;
	};

	/////////////////////////////////////////////////////////
	// Door Component
	// 문짝 컴포넌트, 잠금해제상태라면 플레이어가 직접 열 수 있다.
	//
	class DoorControl : public ComponentBase<DoorControl> {
		float m_MaxAngle = 120.0f;
		float m_CurrentAngle = 0.0f;

		bool m_Locked = true;
		int m_Answer = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool IsLocked() const { return m_Locked; }
		float GetMaxAngle() const { return m_MaxAngle; }
		float GetCurAngle() const { return m_CurrentAngle; }
		int GetAnswer() const { return m_Answer; }

		void SetLock(bool lock) { m_Locked = lock; }
		void SetMaxAngle(float angle) { m_MaxAngle = angle; }
		void SetCurAngle(float angle) { m_CurrentAngle = angle; }

	};

	/////////////////////////////////////////////////////////
	// Throwable Component
	// 던질 수 있는 오브젝트에 사용, 마우스 홀드로 쿠킹 후 투척
	//
	class Throwable : public ComponentBase<Throwable> {

	};

	/////////////////////////////////////////////////////////
	// Attackable Component
	// 공격할 수 있는 오브젝트에 사용, 
	//
	class Attackable : public ComponentBase<Attackable> {

	};

	/////////////////////////////////////////////////////////
	// Holdable Component
	// 손에 들 수 있는 오브젝트(손에 들 수 있다)
	//
	class Holdable : public ComponentBase<Holdable> {
		// 아래는 보류하자
		//bool m_HoldableEnable = true;	// after throw, disable
		bool m_CurrentHolding = false;

		bool m_Holding = false;
		std::function<void()> m_OnMouseClicked;
	};

#define MAX_INVENTORY 4
	/////////////////////////////////////////////////////////
	// Inventory Component
	// Holdable 들을 보관한다
	//
	class Inventory : public ComponentBase<Inventory> {
		Entity* m_Items[MAX_INVENTORY] = {nullptr, };
		const char* m_TargetEntityNames[MAX_INVENTORY] = { nullptr, };
		int m_CurrentHolding = 0;

		// anim socket
		Entity* m_HoldingSocket = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		Entity* GetCurrentHoldingItem() const { return m_Items[m_CurrentHolding]; }
		
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// UI 관련 컴포넌트들
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////
	// UIKeypad component
	// 문열기 미니게임의 결과
	//
	class UIKeypad : public ComponentBase<UIKeypad> {
		Entity* m_DoorEntity = nullptr;
		int m_Answer = 0;
		int m_Current = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		Entity* GetDoor() { return m_DoorEntity; }

		void SetDoor(Entity* door) { m_DoorEntity = door; }
		void SetAnswer(int ans) { m_Answer = ans; }
		int GetAnswer() const { return m_Answer; }

		void SetCurrent(int Current) { m_Current = Current; }
		int GetCurrent() const { return m_Current; }
	};
}
