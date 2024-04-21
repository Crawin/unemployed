#pragma once

class ECSManager;

namespace ECSsystem {

	// system class's member variable == global setting
	// todo json으로 system 넣기

	/////////////////////////////////////////////////////////
	// bass component system
	//
	class System
	{
	public:
		virtual void OnInit(ECSManager* manager) {}
		virtual void Update(ECSManager* manager, float deltaTime) = 0;
	};

	/////////////////////////////////////////////////////////
	// Transform to its children
	//
	class LocalToWorldTransform : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// animation playtime add
	//
	class AnimationPlayTimeAdd : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// transform to renderer / camera / light
	//
	class SyncWithTransform : public System {

	public:
		virtual void OnInit(ECSManager* manager);
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// sync position from server data
	//
	class SyncPosition : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// input to update physics
	//
	class UpdateInput : public System {

	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// move by physics
	//
	class SimulatePhysics : public System {
		float m_friction = 200.0f;
		float m_Gravity = 9.6f;
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};


	/////////////////////////////////////////////////////////
	// anim set by speed
	//
	class ChangeAnimationTest : public System {

	public:
		virtual void OnInit(ECSManager* manager);
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// rotate directional light by time
	//
	class DayLight : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// collide check only
	//
	class CollideCkeck : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	//class SendToServer : public System {
	//public:
	//	virtual void Update(ECSManager* manager, float deltaTime);
	//};
}


