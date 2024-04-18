﻿#include "Enums.h"
#include <string>

//enum class ANIMATION_STATE {
//	IDLE,
//	WALK,
//	WAIT,
//	FALLINGDOWN,
//	GETUP,
//	DANCE
//};

ANIMATION_STATE ConvertStringToAnimationState(const std::string& str)
{
	if (str == "Wait")				return ANIMATION_STATE::WAITEND;
	if (str == "Idle")				return ANIMATION_STATE::IDLE;
	if (str == "Walk")				return ANIMATION_STATE::WALK;
	if (str == "FallingDown")		return ANIMATION_STATE::FALLINGDOWN;
	if (str == "GetUp")				return ANIMATION_STATE::GETUP;
	if (str == "Dance")				return ANIMATION_STATE::DANCE;

	return ANIMATION_STATE::NULLANIM;
}

const std::string& ConvertAnimationStateToString(ANIMATION_STATE anim)
{
	switch (anim) {
	case ANIMATION_STATE::WAITEND:				return "Wait";
	case ANIMATION_STATE::IDLE:				return "Idle";
	case ANIMATION_STATE::WALK:				return "Walk";
	case ANIMATION_STATE::FALLINGDOWN:		return "FallingDown";
	case ANIMATION_STATE::GETUP:				return "GetUp";
	case ANIMATION_STATE::DANCE:				return "Dance";
	}
}