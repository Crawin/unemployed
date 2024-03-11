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
	class LocalToWorldTransform : public System{
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
	// simple physics (friction)
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


}


