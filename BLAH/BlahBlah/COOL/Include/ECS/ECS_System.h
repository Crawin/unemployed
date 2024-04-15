#pragma once

class ECSManager;

namespace ECSsystem {

	/////////////////////////////////////////////////////////
	// bass component system
	//
	class System
	{
	public:

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
	// transform to renderer / camera
	//
	class SyncWithTransform : public System {

	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// simple physics (friction on air?)
	//
	class Friction : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// move by input
	//
	class MoveByInput : public System {

	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// anim set by speed
	//
	class ChangeAnimationTest : public System {

	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// rotate directional light by time
	//
	class DayLight : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};


}


