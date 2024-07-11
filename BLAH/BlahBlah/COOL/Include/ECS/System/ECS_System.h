#pragma once

class ECSManager;
class ITimeLine;
class Entity;

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
		float m_friction = 600.0f;
		float m_Gravity = 9.6f;
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

	/////////////////////////////////////////////////////////
	// collide check, handle, and event
	//
	class CollideHandle : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);

		static bool CheckCollisionRectCircle(const BoundingOrientedBox& rect, const XMFLOAT2& circleCenter, float circleRadius) {
			XMFLOAT2 rectCenter = { rect.Center.x, rect.Center.y };
			XMFLOAT2 rectSize = { rect.Extents.x, rect.Extents.y };

			float radPow = circleRadius * circleRadius;

			// 반지름으로 사각형 확장
			XMFLOAT2 newSize = { rectSize.x + circleRadius, rectSize.y + circleRadius };

			float left = rectCenter.x - newSize.x;
			float top = rectCenter.y + newSize.y;
			float right = rectCenter.x + newSize.x;
			float bottom = rectCenter.y - newSize.y;

			// is in expend area?
			if ((left < circleCenter.x && circleCenter.x < right &&
				bottom < circleCenter.y && circleCenter.y < top) == false) return false;

			// if in left top
			if (circleCenter.x < left && circleCenter.y > top &&
				pow(circleCenter.x - left, 2) + pow(circleCenter.y - top, 2) > radPow)
				return false;

			// right top
			if (circleCenter.x > right && circleCenter.y > top &&
				pow(circleCenter.x - right, 2) + pow(circleCenter.y - top, 2) > radPow)
				return false;

			// left bottom
			if (circleCenter.x < left && circleCenter.y < bottom &&
				pow(circleCenter.x - left, 2) + pow(circleCenter.y - bottom, 2) > radPow)
				return false;

			// right bottom
			if (circleCenter.x > right && circleCenter.y < bottom &&
				pow(circleCenter.x - right, 2) + pow(circleCenter.y - bottom, 2) > radPow)
				return false;


			return true;
		}


		static bool CheckCollisionRectCircleNoUse(const XMFLOAT2& rectCenter, const XMFLOAT2& rectSize, const XMFLOAT2& circleCenter, float circleRadius) {
			// 반지름으로 사각형 확장
			XMFLOAT2 newSize = { rectSize.x + circleRadius, rectSize.y + circleRadius };

			float radPow = circleRadius * circleRadius;

			float left = rectCenter.x - newSize.x;
			float top = rectCenter.y + newSize.y;
			float right = rectCenter.x + newSize.x;
			float bottom = rectCenter.y - newSize.y;

			// is in expend area?
			if ((left < circleCenter.x && circleCenter.x < right &&
				bottom < circleCenter.y && circleCenter.y < top) == false) return false;

			// 대각선 영역
			left = rectCenter.x - rectSize.x;
			top = rectCenter.y + rectSize.y;
			right = rectCenter.x + rectSize.x;
			bottom = rectCenter.y - rectSize.y;
			
			// if in left top
			if (circleCenter.x < left && circleCenter.y > top &&
				pow(circleCenter.x - left, 2) + pow(circleCenter.y - top, 2) > radPow)
				return false;

			// right top
			if (circleCenter.x > right && circleCenter.y > top &&
				pow(circleCenter.x - right, 2) + pow(circleCenter.y - top, 2) > radPow)
				return false;

			// left bottom
			if (circleCenter.x < left && circleCenter.y < bottom &&
				pow(circleCenter.x - left, 2) + pow(circleCenter.y - bottom, 2) > radPow)
				return false;

			// right bottom
			if (circleCenter.x > right && circleCenter.y < bottom &&
				pow(circleCenter.x - right, 2) + pow(circleCenter.y - bottom, 2) > radPow)
				return false;


			return true;
		}

	};

	/////////////////////////////////////////////////////////
	// move y physics, end on last
	//
	class MoveByPhysics : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// interaction
	// 
	class HandleInteraction : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// handle UI components
	// 
	class HandleUIComponent : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// Send To Server
	// 
	class SendToServer : public System {
	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// TimeLine Managing
	// 
	class TimeLineManaging : public System {
		std::map<Entity*, ITimeLine*> m_TimeLines;

	public:
		virtual void Update(ECSManager* manager, float deltaTime);
	};
}


