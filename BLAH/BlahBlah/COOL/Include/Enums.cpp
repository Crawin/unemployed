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
	if (str == "Wait")				return ANIMATION_STATE::WAITEND;
	if (str == "Idle")				return ANIMATION_STATE::IDLE;
	if (str == "Walk")				return ANIMATION_STATE::WALK;
	if (str == "Run")				return ANIMATION_STATE::RUN;
	if (str == "FallingDown")		return ANIMATION_STATE::FALLINGDOWN;
	if (str == "GetUp")				return ANIMATION_STATE::GETUP;
	if (str == "Dance")				return ANIMATION_STATE::DANCE;

	return ANIMATION_STATE::NULLANIM;
}

std::string ConvertAnimationStateToString(ANIMATION_STATE anim)
{
	switch (anim) {
	case ANIMATION_STATE::WAITEND:				return std::string("Wait");
	case ANIMATION_STATE::IDLE:					return std::string("Idle");
	case ANIMATION_STATE::WALK:					return std::string("Walk");
	case ANIMATION_STATE::RUN:					return std::string("RUN");
	case ANIMATION_STATE::FALLINGDOWN:			return std::string("FallingDown");
	case ANIMATION_STATE::GETUP:				return std::string("GetUp");
	case ANIMATION_STATE::DANCE:				return std::string("Dance");
	}
}

// global key setting
int ConvertGameInputEnumToKeyIntValue(GAME_INPUT gameInput)
{
	switch (gameInput)
	{
	case GAME_INPUT::FORWARD:				 return 'W';
	case GAME_INPUT::BACKWARD:				 return 'S';
	case GAME_INPUT::LEFT:					 return 'A';
	case GAME_INPUT::RIGHT:					 return 'D';
	case GAME_INPUT::INTERACTION:			 return 'E';
	case GAME_INPUT::CTRL:					 return VK_CONTROL;
	case GAME_INPUT::SHIFT:					 return VK_SHIFT;
	case GAME_INPUT::SPACE_BAR:				 return VK_SPACE;
	case GAME_INPUT::MOUSE_LEFT:			 return VK_LBUTTON;
	case GAME_INPUT::MOUSE_RIGHT:			 return VK_RBUTTON;
	case GAME_INPUT::NUM_1:					 return '1';
	case GAME_INPUT::NUM_2:					 return '2';
	case GAME_INPUT::NUM_3:					 return '3';
	case GAME_INPUT::NUM_4:					 return '4';
	case GAME_INPUT::NUM_5:					 return '5';
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
	case PRE_LOADED_MATERIALS::BLIT:			return std::string("_Blit");

#ifdef _DEBUG
	case PRE_LOADED_MATERIALS::FOR_DEBUG:		return std::string("_ForDebug");
#endif
	}

	// 이즈리얼이 사는 곳은?
	// 비전2동
	ERROR_QUIT(std::format("ERROR!!!! no match materials, idx: {}", static_cast<int>(preLaodMat)));
}
