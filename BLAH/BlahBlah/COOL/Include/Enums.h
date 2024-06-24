﻿#pragma once


enum class ROOT_SIGNATURE_IDX{
	DESCRIPTOR_HEAP = 0,			// 디스크립터힙
	DESCRIPTOR_IDX_CONSTANT,		// 디스크립터를 사용할 인덱스 16개의 int
	CAMERA_DATA_CBV,				// 카메라행렬 4x4 x2
	WORLD_MATRIX,
	SHADER_DATAS_CBV,				// 뭐 대충 나머지들 (delta time, 등등)
	//BONE,
	//ANIMATION_FIRST,
	//ANIMATION_SECOND,				// 늘릴 필요 없었다.
	ROOT_SIGNATURE_IDX_MAX
};

enum class MATERIAL_TYPES {
	ALBEDO = 0,
	ROUGHNESS,
	METALIC,
	SPECULAR,
	NORMAL,
	MATERIAL_END
};

enum class ANIMATION_STATE {
	WAITEND = 0,
	IDLE,
	WALK,
	RUN,
	FALLINGDOWN,
	GETUP,
	DANCE,
	NULLANIM
};

enum class KEY_STATE {
	NONE = 0,
	START_PRESS,
	PRESSING,
	END_PRESS
};

enum class GAME_INPUT {
	FORWARD = 0,
	BACKWARD,
	LEFT,
	RIGHT,
	INTERACTION,
	CTRL,
	SHIFT,
	SPACE_BAR,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	GAME_INPUT_END
};



ANIMATION_STATE ConvertStringToAnimationState(const std::string& str);
std::string ConvertAnimationStateToString(ANIMATION_STATE anim);
int ConvertGameInputEnumToKeyIntValue(GAME_INPUT gameInput);

// struct
//struct PS_MRT_OUTPUT
//{
//	float4 Albedo : SV_TARGET0;
//	float4 Roughness : SV_TARGET1;
//	float4 Metalic : SV_TARGET2;
//	float4 Specular : SV_TARGET3;
//	float4 NormalW : SV_TARGET4;
//	float4 PositionW : SV_Target5;
//};

// PBR
// MATERIAL_TYPES랑 같이 쓸까?
// ㄴㄴ
enum class MULTIPLE_RENDER_TARGETS {
	BaseColor = 0,
	Roughness,
	Metalic,
	Specular,
	NormalW,
	PositionW,
	MRT_END
};

enum class MRT_POST_ROOTCONST
{
	ALBEDO,
	ROUGHNESS,
	METALIC,
	SPECULAR,
	NORMAL,
	POSITION,
	LIGHT_SIZE,
	LIGHT_IDX,
};

enum class VERTEX_TYPES {
	NO_VERTEX = 0,
	NORMAL,
	SKINNED
};

enum class RESOURCE_TYPES {
	SHADER = 0,
	VERTEX,
	OBJECT,
};

enum class ANIM_ROOTCONST {
	ANI_1_PLAYTIME = 0,
	ANI_2_PLAYTIME,
	ANI_BLEND,
	ANI_1_FRAME,
	ANI_2_FRAME,
	ANI_1_INDEX,
	ANI_2_INDEX,
	BONE_INDEX
};

enum class DAYLIGHT_ROOTCONST {
	DAY_LIGHT = 0,
	MOON_LIGHT = 4,
	SUNSET_LIGHT = 8,
	LIGHT_RESOURCE_INDEX = 12,
	MAIN_LIGHT_INDEX = 13,
	LIGHT_ANGLE = 14,
};

enum class UI_ROOT_CONST {
	TEXTURE_IDX = 0,
	SPRITE_SIZE = 1,
	CUR_SPRITE = 3,

	UI_CENTER = 4,
	UI_SIZE = 6,

	SCREEN_SIZE = 8,
	
	//TEXTURE_IDX = 0,
	//SPRITE_X_SIZE,
	//SPRITE_Y_SIZE,
	//CUR_SPRITE,

	//UI_X_CENTER,
	//UI_Y_CENTER,
	//UI_WIDTH,
	//UI_HEIGHT,

	//SCREEN_WIDTH,
	//SCREEN_HEIGHT
};



enum class GAME_STATE {
	MAIN_STATE,
	UI_TOP_STATE
};
