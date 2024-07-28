#include "framework.h"
#include "Enums.h"
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
	if (str == "Wait")								return ANIMATION_STATE::WAITEND;
	if (str == "Idle")								return ANIMATION_STATE::IDLE;
	if (str == "Walk")								return ANIMATION_STATE::WALK;
	if (str == "Run")								return ANIMATION_STATE::RUN;
	if (str == "Blended_Moving_State")				return ANIMATION_STATE::BLENDED_MOVING_STATE;
	if (str == "Forward_Walk")						return ANIMATION_STATE::FORWARD_WALK;
	if (str == "Forward_Run")						return ANIMATION_STATE::FORWARD_RUN;
	if (str == "Backward_Walk")						return ANIMATION_STATE::BACKWARD_WALK;
	if (str == "Backward_Run")						return ANIMATION_STATE::BACKWARD_RUN;
	if (str == "Left_Walk_Strafe")					return ANIMATION_STATE::LEFT_WALK_STRAFE;
	if (str == "Left_Run_Strafe")					return ANIMATION_STATE::LEFT_RUN_STRAFE;
	if (str == "Right_Walk_Strafe")					return ANIMATION_STATE::RIGHT_WALK_STRAFE;
	if (str == "Right_Run_Strafe")					return ANIMATION_STATE::RIGHT_RUN_STRAFE;
	if (str == "Jump_Start")						return ANIMATION_STATE::JUMP_START;
	if (str == "Jump_Ing")							return ANIMATION_STATE::JUMP_ING;
	if (str == "Jump_Land")							return ANIMATION_STATE::JUMP_LAND;
	if (str == "Getting_Hit")						return ANIMATION_STATE::GETTING_HIT;
	if (str == "GetUp")								return ANIMATION_STATE::GETUP;
	if (str == "Dance")								return ANIMATION_STATE::DANCE;
	if (str == "Sit_Start")							return ANIMATION_STATE::SIT_START;
	if (str == "Sit_Loop")							return ANIMATION_STATE::SIT_LOOP;
	if (str == "Sit_End")							return ANIMATION_STATE::SIT_END;
	if (str == "Attack")							return ANIMATION_STATE::ATTACK;
	if (str == "Throw_Start")						return ANIMATION_STATE::THROW_START;
	if (str == "Throw_End")							return ANIMATION_STATE::THROW_END;

	return ANIMATION_STATE::NULLANIM;
}

std::string ConvertAnimationStateToString(ANIMATION_STATE anim)
{
	switch (anim) {
	case ANIMATION_STATE::WAITEND:					return std::string("Wait");
	case ANIMATION_STATE::IDLE:						return std::string("Idle");
	case ANIMATION_STATE::WALK:						return std::string("Walk");
	case ANIMATION_STATE::RUN:						return std::string("Run");
	case ANIMATION_STATE::BLENDED_MOVING_STATE:		return std::string("Blended_Moving_State");
	case ANIMATION_STATE::FORWARD_WALK:				return std::string("Forward_Walk");
	case ANIMATION_STATE::FORWARD_RUN:				return std::string("Forward_Run");
	case ANIMATION_STATE::BACKWARD_WALK:			return std::string("Backward_Walk");
	case ANIMATION_STATE::BACKWARD_RUN:				return std::string("Backward_Run");
	case ANIMATION_STATE::LEFT_WALK_STRAFE:			return std::string("Left_Walk_Strafe");
	case ANIMATION_STATE::LEFT_RUN_STRAFE:			return std::string("Left_Run_Strafe");
	case ANIMATION_STATE::RIGHT_WALK_STRAFE:		return std::string("Right_Walk_Strafe");
	case ANIMATION_STATE::RIGHT_RUN_STRAFE:			return std::string("Right_Run_Strafe");
	case ANIMATION_STATE::JUMP_START:				return std::string("Jump_Start");
	case ANIMATION_STATE::JUMP_ING:					return std::string("Jump_Ing");
	case ANIMATION_STATE::JUMP_LAND:				return std::string("Jump_Land");
	case ANIMATION_STATE::GETTING_HIT:				return std::string("Getting_Hit");
	case ANIMATION_STATE::GETUP:					return std::string("GetUp");
	case ANIMATION_STATE::DANCE:					return std::string("Dance");
	case ANIMATION_STATE::SIT_START:				return std::string("Sit_Start");
	case ANIMATION_STATE::SIT_LOOP:					return std::string("Sit_Loop");
	case ANIMATION_STATE::SIT_END:					return std::string("Sit_End");
	case ANIMATION_STATE::ATTACK:					return std::string("Attack");
	case ANIMATION_STATE::THROW_START:				return std::string("Throw_Start");
	case ANIMATION_STATE::THROW_END:				return std::string("Throw_End");
	}
}

// global key setting
int ConvertGameInputEnumToKeyIntValue(GAME_INPUT gameInput)
{
	switch (gameInput)
	{
	case GAME_INPUT::FORWARD:				return 'W';
	case GAME_INPUT::BACKWARD:				return 'S';
	case GAME_INPUT::LEFT:					return 'A';
	case GAME_INPUT::RIGHT:					return 'D';
	case GAME_INPUT::INTERACTION:			return 'E';
	case GAME_INPUT::CTRL:					return VK_CONTROL;
	case GAME_INPUT::SHIFT:					return VK_SHIFT;
	case GAME_INPUT::SPACE_BAR:				return VK_SPACE;
	case GAME_INPUT::MOUSE_LEFT:			return VK_LBUTTON;
	case GAME_INPUT::MOUSE_RIGHT:			return VK_RBUTTON;
	case GAME_INPUT::NUM_1:					return '1';
	case GAME_INPUT::NUM_2:					return '2';
	case GAME_INPUT::NUM_3:					return '3';
	case GAME_INPUT::NUM_4:					return '4';
	case GAME_INPUT::NUM_5:					return '5';
	case GAME_INPUT::NUM_6:					return '6';
	case GAME_INPUT::ARROW_UP:				return VK_UP;
	case GAME_INPUT::ARROW_DOWN:			return VK_DOWN;
	case GAME_INPUT::ARROW_LEFT:			return VK_LEFT;
	case GAME_INPUT::ARROW_RIGHT:			return VK_RIGHT;
	case GAME_INPUT::NEXT:					return '[';
	case GAME_INPUT::PREVIOUS:				return ']';
	case GAME_INPUT::F1:					return VK_F1;
	case GAME_INPUT::F2:					return VK_F2;
	case GAME_INPUT::F3:					return VK_F3;
	case GAME_INPUT::F4:					return VK_F4;
	case GAME_INPUT::F5:					return VK_F5;
	case GAME_INPUT::F6:					return VK_F6;
	case GAME_INPUT::F7:					return VK_F7;
	case GAME_INPUT::F8:					return VK_F8;
	case GAME_INPUT::F9:					return VK_F9;
	case GAME_INPUT::F10:					return VK_F10;
	case GAME_INPUT::F11:					return VK_F11;
	case GAME_INPUT::F12:					return VK_F12;

	default:
		DebugPrint("ERRPR!! no such input defined");
		return -1;
	}
}

PRE_LOADED_MATERIALS ConvertStringToMaterial(const std::string& str)
{
	if (str == "_Lighting")				return PRE_LOADED_MATERIALS::LIGHTING;
	if (str == "_ShadowMapping")		return PRE_LOADED_MATERIALS::SHADOWMAPPING;
	if (str == "_Blit")					return PRE_LOADED_MATERIALS::BLIT;
	if (str == "_NoiseCCTV")			return PRE_LOADED_MATERIALS::NOISE_CCTV;
	if (str == "_NoiseNormal")			return PRE_LOADED_MATERIALS::NOISE_NORMAL;
	if (str == "_Vignette")				return PRE_LOADED_MATERIALS::VIGNETTING;


#ifdef _DEBUG
	if (str == "_ForDebug")				return PRE_LOADED_MATERIALS::FOR_DEBUG;
#endif

	// 길에 넘어지는걸 영어로 하면?
	// 다운로드
	ERROR_QUIT(std::format("ERROR!!!! no material match names, name: {}, ", str));
}

std::string ConvertMaterialToString(PRE_LOADED_MATERIALS preLaodMat)
{
	switch (preLaodMat) {
	case PRE_LOADED_MATERIALS::LIGHTING:		return std::string("_Lighting");
	case PRE_LOADED_MATERIALS::SHADOWMAPPING:	return std::string("_ShadowMapping");
	case PRE_LOADED_MATERIALS::OUTLINE:			return std::string("_OutLine");
	case PRE_LOADED_MATERIALS::PARTICLE:		return std::string("_Particle");
	case PRE_LOADED_MATERIALS::BLIT:			return std::string("_Blit");
	case PRE_LOADED_MATERIALS::NOISE_CCTV:		return std::string("_NoiseCCTV");
	case PRE_LOADED_MATERIALS::NOISE_NORMAL:	return std::string("_NoiseNormal");
	case PRE_LOADED_MATERIALS::VIGNETTING:		return std::string("_Vignette");

#ifdef _DEBUG
	case PRE_LOADED_MATERIALS::FOR_DEBUG:		return std::string("_ForDebug");
#endif
	}

	// 이즈리얼이 사는 곳은?
	// 비전2동
	ERROR_QUIT(std::format("ERROR!!!! no match materials, idx: {}", static_cast<int>(preLaodMat)));
}

PARTICLE_TYPES ConvertStringToParticleType(const std::string& str)
{
	if (str == "Spark")				return PARTICLE_TYPES::SPARK;
	if (str == "Water")				return PARTICLE_TYPES::WATER;

	return PARTICLE_TYPES::PARTICLE_TYPE_END;
}

std::string ConvertParticleTypeToString(PARTICLE_TYPES type)
{
	switch (type) {
	case PARTICLE_TYPES::SPARK:		return std::string("Spark");
	case PARTICLE_TYPES::WATER:		return std::string("Water");
	}

	ERROR_QUIT("ERROR");
}
