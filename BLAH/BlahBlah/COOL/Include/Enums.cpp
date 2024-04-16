#include "Enums.h"
#include <string>

//enum class ANIMATION_SET {
//	IDLE,
//	WALK,
//	WAIT,
//	FALLINGDOWN,
//	GETUP,
//	DANCE
//};

ANIMATION_SET ConvertStringToAnimationSet(const std::string& str)
{
	if (str == "Wait")				return ANIMATION_SET::WAITEND;
	if (str == "Idle")				return ANIMATION_SET::IDLE;
	if (str == "Walk")				return ANIMATION_SET::WALK;
	if (str == "FallingDown")		return ANIMATION_SET::FALLINGDOWN;
	if (str == "GetUp")				return ANIMATION_SET::GETUP;
	if (str == "Dance")				return ANIMATION_SET::DANCE;

	return ANIMATION_SET::NULLANIM;
}

const std::string& ConvertAnimationSetToString(ANIMATION_SET anim)
{
	switch (anim) {
	case ANIMATION_SET::WAITEND:				return "Wait";
	case ANIMATION_SET::IDLE:				return "Idle";
	case ANIMATION_SET::WALK:				return "Walk";
	case ANIMATION_SET::FALLINGDOWN:		return "FallingDown";
	case ANIMATION_SET::GETUP:				return "GetUp";
	case ANIMATION_SET::DANCE:				return "Dance";
	}
}