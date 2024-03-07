#pragma once

namespace ECSsystem {

	/////////////////////////////////////////////////////////
	// bass component system
	//
	class System
	{
	public:

		virtual void Update(float deltaTime) = 0;
	};

	/////////////////////////////////////////////////////////
	// move by input
	//
	class MoveByInput : public System {

	public:
		virtual void Update(float deltaTime);
	};


}


