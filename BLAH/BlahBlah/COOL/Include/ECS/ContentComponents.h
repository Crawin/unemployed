#pragma once

#include "Component.h"

struct Input_State_In_LongLong
{
	long long int m_Data;

	Input_State_In_LongLong(GAME_INPUT input, KEY_STATE keyState) {
		long long int forward = static_cast<long long int>(input);
		long long int backward = static_cast<long long int>(keyState);
		forward <<= 32;

		m_Data = forward | backward;
	}

	GAME_INPUT GetInput() const { return static_cast<GAME_INPUT>(m_Data >> 32); }
	KEY_STATE GetState() const { return static_cast<KEY_STATE>(m_Data << 32 >> 32); }

	auto operator<=>(const Input_State_In_LongLong& other) const = default;
};

namespace std {
	template<>
	struct hash<Input_State_In_LongLong> {
		std::size_t operator()(const Input_State_In_LongLong& k) const {
			return std::hash<long long int>()(k.m_Data);
		}
	};
}

// void func(float deltaTime);
using ActionFunction = std::function<void(float)>;
using ActionFunctionMap = std::unordered_map<Input_State_In_LongLong, ActionFunction>;

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
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

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
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

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
		int m_Length = 0;


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

	class DoorKey : public ComponentBase<DoorKey> {
		Entity* m_DoorEntity = nullptr;
		int m_Length = 0;
		int m_Current = 0;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		Entity* GetDoor() { return m_DoorEntity; }

		void SetDoor(Entity* door) { m_DoorEntity = door; }
		void SetAnswer(int len) { m_Length = len; }
		int GetAnswer() const { return m_Length; }

	};

	/////////////////////////////////////////////////////////
	// Holdable Component
	// 손에 들 수 있는 오브젝트(손에 들 수 있다)
	//
	class Holdable : public ComponentBase<Holdable> {
		ActionFunctionMap m_ActionMap;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		const ActionFunctionMap& GetActionMap() const { return m_ActionMap; }

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Attackable Component
	//
	//
	class Attackable : public ComponentBase<Attackable> {
		ActionFunctionMap m_ActionMap;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

#define MAX_INVENTORY 4
	/////////////////////////////////////////////////////////
	// Inventory Component
	// Holdable 들을 보관한다
	//
	class Inventory : public ComponentBase<Inventory> {
		Entity* m_Items[MAX_INVENTORY] = {nullptr, };
		std::string m_TargetEntityNames[MAX_INVENTORY] = { "", };
		//const char* m_TargetEntityNames[MAX_INVENTORY] = { nullptr, };
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
