#pragma once

#include "Component.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 컨텐츠 관련 컴포넌트들
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace component {
	/////////////////////////////////////////////////////////
// Door Component
// 문짝 컴포넌트, 잠금해제상태라면 플레이어가 직접 열 수 있다.
//
	class DoorControl : public ComponentBase<DoorControl> {
		float m_MaxAngle = 120.0f;
		float m_CurrentAngle = 0.0f;
		bool m_Locked = true;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

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
}
