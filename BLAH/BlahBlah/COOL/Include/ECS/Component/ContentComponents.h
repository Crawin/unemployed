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

	class Transform;
	/////////////////////////////////////////////////////////
	// Door Component
	// 문짝 컴포넌트, 잠금해제상태라면 플레이어가 직접 열 수 있다.
	//
	class DoorControl : public ComponentBase<DoorControl> {
		float m_MaxAngle = 120.0f;
		float m_CurrentAngle = 0.0f;

		bool m_Locked = true;
		bool m_Uioff = false;
		bool m_Open = false;
		bool m_KeyDoorOpen = false;
		int m_Answer = 0;
		int m_Gamemode = -1;
		int m_FailCount = 0;
		int m_KeyID = -1;
		int m_Hp = 2;

		// 0, 1, 2 -> x, y, z
		int m_RotateAxis = 1;

		XMFLOAT4 m_CrushPositionAndPower[2] = { {0,0,0,0}, {0,0,0,0} };
		int m_NextCrushIndex = 0;

		int m_DoorId = -1;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool IsLocked() const { return m_Locked; }
		float GetMaxAngle() const { return m_MaxAngle; }
		float GetCurAngle() const { return m_CurrentAngle; }
		int GetAnswer() const { return m_Answer; }
		int GetKeyID() const { return m_KeyID; }
		int GetGamemode() const { return m_Gamemode; }
		int GetFailCount() const { return m_FailCount; }
		int IsUioff() const { return m_Uioff; }
		bool IsOpen() const { return m_Open; }
		int GetAxis() const { return m_RotateAxis; }
		int GetDoorID() const { return m_DoorId; }
		bool GetKeyDoorOpen() const { return m_KeyDoorOpen; }
		int GetDoorHp() const { return m_Hp; }

		void SetLock(bool lock, bool sendServer = true);
		void SetMaxAngle(float angle) { m_MaxAngle = angle; }
		void SetCurAngle(float angle) { m_CurrentAngle = angle; }
		void SetUioff(bool uioff) { m_Uioff = uioff; }
		void SetOpen(ECSManager* manager, Transform* tr, Entity* self, bool state, bool sendServer = true);
		//void SetOpen(bool state) { m_Open = state; }
		void SetKeyDoorOpen(bool keyopen) { m_KeyDoorOpen = keyopen; }
		void SetDoorHp(int hp) { m_Hp = hp; }
		void SetAnswer(int answer) { m_Answer = answer; }

		void SetCrushPosition(XMFLOAT3 pos, float power, float distance);
		XMFLOAT4 GetShaderData(int idx) const { return m_CrushPositionAndPower[idx]; }
	};


	/////////////////////////////////////////////////////////
	// Holdable Component
	// 손에 들 수 있는 오브젝트(손에 들 수 있다)
	//
	class Holdable : public ComponentBase<Holdable> {
		ActionFunctionMap m_ActionMap;

		// entity를 들고 있는 주체
		Entity* m_HoldingMaster = nullptr;

		// 원래 있던 곳, (사용하지 않는 Holdable들은 안보이는곳에 보관
		Entity* m_OriginalParent = nullptr;

		int m_HoldableItemID = -1;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		void SetAction(const Input_State_In_LongLong& keyCond, const ActionFunction& act) { m_ActionMap[keyCond] = act; }
		const ActionFunctionMap& GetActionMap() const { return m_ActionMap; }

		void SetMaster(Entity* master) { m_HoldingMaster = master; }
		Entity* GetMaster() const { return m_HoldingMaster; }
		Entity* GetOriginParent() const { return m_OriginalParent; }

		int GetHoldableID() const { return m_HoldableItemID; }

		virtual void ShowYourself() const {};
	};

	/////////////////////////////////////////////////////////
	// Attackable Component
	// 공격 아이템 
	//
	class Attackable : public ComponentBase<Attackable> {

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Throwable Component
	// CCTV, drinks
	//
	class Throwable : public ComponentBase<Throwable> {
		XMFLOAT3 m_DirectionOrigin = { 0.0f, 0.0f, 1.0f };
		XMFLOAT3 m_DirectionResult = { 0.0f, 0.0f, 1.0f };
		float m_ThrowBakeTime = 0.0f;
		float m_ThrowBakeTimeMax = 3.0f;
		float m_ThrowSpeed = 2000.0f;			// abuot 70km/h, our unit ->cm/s

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

	/////////////////////////////////////////////////////////
	// Consumable Component
	// CCTV, drinks
	//
	class Consumable : public ComponentBase<Consumable> {
		int m_ConsumType = -1;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

#define MAX_CAMERA 4
	/////////////////////////////////////////////////////////
	// Throwable Component
	// CCTV, drinks
	//
	class CCTVController : public ComponentBase<CCTVController> {
		Entity* m_CCTVEntities[MAX_CAMERA] = {};
		std::string m_TargetCCTVNames[MAX_CAMERA] = { "", };

		int m_CurrentCCTV = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

#define MAX_INVENTORY 6
	/////////////////////////////////////////////////////////
	// Inventory Component
	// Holdable 들을 보관한다
	//
	class Inventory : public ComponentBase<Inventory> {
		Entity* m_Items[MAX_INVENTORY] = {nullptr, };

		std::string m_InventorySocketName;
		std::string m_SubInventorySocketName;
		std::string m_TargetEntityNames[MAX_INVENTORY] = { "", };
		//const char* m_TargetEntityNames[MAX_INVENTORY] = { nullptr, };
		int m_CurrentHolding = 0;

		// anim socket
		Entity* m_HoldingSocket = nullptr;
		Entity* m_SubHoldingSocket = nullptr;

		bool m_MainMode = true;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool ChangeHoldingItem(int idx, ECSManager* manager);

		Entity* GetCurrentHoldingItem() const { return m_Items[m_CurrentHolding]; }
		const Entity* GetHoldingSocket() const;

		void EraseCurrentHolding() { m_Items[m_CurrentHolding] = nullptr; }
		void AddItem(Entity* entity, int idx) { m_Items[idx] = entity; }

		bool IsOccupied(int target) { return m_Items[target] != nullptr; }

		bool IsMainMode() const { return m_MainMode; }
		void SetMainMode(bool state, ECSManager* manager);
	};

	/////////////////////////////////////////////////////////
	// Sittable Component
	// 앉을 수 있는 곳
	//
	class Sittable : public ComponentBase<Sittable> {
		XMFLOAT3 m_AttachPosition = { 0,0,0 };
		Entity* m_SittableSocket = nullptr;

		std::string m_SittableSocketName;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

	/////////////////////////////////////////////////////////
	// VandingMachine Component
	// 자판기 내용물 보관
	//
	class VandingMachine : public ComponentBase<VandingMachine> {

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

	/////////////////////////////////////////////////////////
	// Drink Component
	// 음료 속성 보관
	//
#define DEFAULT_SOCKET 3
#define MAX_DRINK_TYPE 3
	class Drink : public ComponentBase<Drink> {
		bool m_Occupied = false;
		int m_Type = 0;
		int m_InventorySocket = DEFAULT_SOCKET;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetOccupied(bool state) { m_Occupied = state; }
		bool isOccupied() const { return m_Occupied; }

		int GetType() const { return m_Type; }
		int GetInvenSocket() const { return m_InventorySocket; }
	};

	/////////////////////////////////////////////////////////
	// CreditCard Component
	// 돈 내용물 보관
	//
	class CreditCard : public ComponentBase<CreditCard> {
		int m_Money = 1000;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

#define MAX_KEYTOOL_HOLDING 10
	/////////////////////////////////////////////////////////
	// KeyTool Component
	// 열쇠뭉치 컴포넌트
	//
	class KeyTool : public ComponentBase<KeyTool> {
		int m_Keys[MAX_KEYTOOL_HOLDING] = {  };
		int m_SoundMakingMinimum = 3;
		int m_CurrentHolding = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
		
		// if key max; return false;
		bool InsertKey(int keyAnswer);
		
		bool DeleteKey(int keyAnswer);

		bool IsKeyInKeyTool(int keyAnswer) const;

		int GetKeyHold(int slot) const { return m_Keys[slot]; }
	};

#define MAX_CCTV 4
	/////////////////////////////////////////////////////////
	// Screen Component
	// CCTV와 연결되는 스크린
	//
	class Screen : public ComponentBase<Screen> {
		int m_CameraRenderTargets[MAX_CCTV] = { 0, };

		std::string m_TargetNames[MAX_CCTV] = { "" };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Key Component
	// 문을 열 수 있는 열쇠의 정보를 가진 키 오브젝트
	//
	class Key : public ComponentBase<Key> {
		int m_KeyID = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetKeyID(int id) { m_KeyID = id; }

		int GetKeyID() const { return m_KeyID; }
	};

	/////////////////////////////////////////////////////////
	// RCController Component
	// RCController
	//
	class RCController : public ComponentBase<RCController> {
		Entity* m_RCEntity = nullptr;
		std::string m_TargetRC = "";

		float m_GoRotateAngle = 0.0f;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

	/////////////////////////////////////////////////////////
	// StartInteraction Component
	// StartInteraction
	//
	class StartInteraction : public ComponentBase<StartInteraction> {
		bool m_Ready = false;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) {}
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		bool IsReady() const { return m_Ready; }
		void SetReady(bool state) { m_Ready = state; }
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

	/////////////////////////////////////////////////////////
	// UIDoorKey component
	// 열쇠 미니게임의 결과
	//
#define MAX_KEY 2
#define MAX_BUTTON 8
	class UIRenderer;
	class UIDoorKey : public ComponentBase<UIDoorKey> {
		Entity* m_DoorEntity = nullptr;
		int m_Answer = 0;
		int m_Current = 0;
		int m_FailCount = 0;

		std::map<int, int> m_KeyMaterialMap;
		UIRenderer* m_AnswerUIrender = nullptr;
		UIRenderer* m_AnswerbuttonUIrender[MAX_BUTTON] = {};	
		Key* m_keyAnswer[MAX_BUTTON] = {};	// 키 이미지 정답


	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		Entity* GetDoor() { return m_DoorEntity; }

		void SetDoor(Entity* door) { m_DoorEntity = door; }
		void SetAnswer(int ans) { m_Answer = ans; }
		void SetCurrent(int Current) { m_Current = Current; }
		void SetFailCount(int failcnt) { m_FailCount = failcnt; }

		int GetAnswer() const { return m_Answer; }
		int GetCurrent() const { return m_Current; }
		int GetFailCount() const { return m_FailCount; }
		int GetMaterial(int ans) { return m_KeyMaterialMap[ans]; }

		void SetAnswerMaterial(int idx); 	// 정답지 이미지
		void SetAnswerButtonMaterial(int target, int answer); 	//들고 있는 열쇠들 이미지
		void SetAnswerButton(int target, int answer); 
	};

	/////////////////////////////////////////////////////////
	// UIDoorKey component
	// 열쇠 미니게임의 결과
	//
	class UICutLine : public ComponentBase<UICutLine> {
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
		void SetCurrent(int Current) { m_Current = Current; }

		int GetAnswer() const { return m_Answer; }
		int GetCurrent() const { return m_Current; }

	};


	/////////////////////////////////////////////////////////
	// UITreasureChest Component
	// 최종 금고를 열기 위한 그거
	//
#define FINAL_LOCK 5
	class UITreasureChest : public ComponentBase<UITreasureChest> {
		Entity* m_DoorEntity = nullptr;
		
		int m_LockLeft = FINAL_LOCK;
		int m_Answer[FINAL_LOCK] = { -1, -1, -1, -1, -1 };
		int m_Current = 0;

		std::map<int, int> m_KeyMaterialMap;
		UIRenderer* m_AnswerUIrender[FINAL_LOCK] = {};
		UIRenderer* m_AnswerbuttonUIrender[MAX_BUTTON] = {};
		Key* m_keyAnswer[MAX_BUTTON] = {};	// 키 이미지 정답

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetDoor(Entity* door) { m_DoorEntity = door; }
		void SetAnswer(int ans);
		void SetCurrent(int Current) { m_Current = Current; }

		int GetCouneLeft() const { return m_LockLeft; }
		void SetLeftCount(int count) { m_LockLeft = count; }
		Entity* GetDoor() const { return m_DoorEntity; }

		void SetAnswerButtonMaterial(int target, int answer); 	//들고 있는 열쇠들 이미지
		void SetAnswerButton(int target, int answer);
	};

	/////////////////////////////////////////////////////////
	// UIVandingMachine Component
	// 자판기 ui, 버튼 누르면 음료 나옴
	//
	class UIVandingMachine : public ComponentBase<UIVandingMachine> {
		Entity* m_PlayerEntity = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetPlayer(Entity* player) { m_PlayerEntity = player; }
	};


#define MAX_ENDINGS 4
	/////////////////////////////////////////////////////////
	// UI Ending Component
	// 자판기 ui, 버튼 누르면 음료 나옴
	//
	class UIEnding : public ComponentBase<UIEnding> {
		UIRenderer* m_EndingImageRenderer;
		
		int m_EndingImageMaterials[MAX_ENDINGS];
		std::string m_EndingImageStrings[MAX_ENDINGS];

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetEndingImage(int endingType);

	};

	/////////////////////////////////////////////////////////
	// UI Conversation Component
	// 대화창
	//
	class UIConversation : public ComponentBase<UIConversation> {
		UIRenderer* m_ConversationUI = nullptr;
		UIRenderer* m_TalkerUI = nullptr;

		std::string* m_ConversationsString = nullptr;
		int m_NumOfConversations = 0;
		std::vector<int> m_ConversationsMaterialIdx;

		std::string* m_TalkersString = nullptr;
		int m_NumOfTalkers = 0;
		std::vector<int> m_TalkersMaterialIdx;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetConversation(int convIdx);
		void SetTalker(int talkerIdx);
	};

	/////////////////////////////////////////////////////////
	// UI Login Component
	// 대화창
	//
	class UILogin : public ComponentBase<UILogin> {
		int m_ToJoinRoom = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) {}
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}

		void SetRoomNum(int num) { m_ToJoinRoom = num; }
	};

}
