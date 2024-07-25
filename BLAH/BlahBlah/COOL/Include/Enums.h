#pragma once


// root signature index
enum class ROOT_SIGNATURE_IDX{
	DESCRIPTOR_HEAP = 0,			// 디스크립터힙
	DESCRIPTOR_IDX_CONSTANT,		// 디스크립터를 사용할 인덱스 16개의 int
	SHADER_EXTRA = 1,
	CAMERA_DATA_CBV,				// 카메라행렬 4x4 x2
	WORLD_MATRIX = 3,
	ANIMATION_EXTRA = 3,
	SHADER_DATAS_CBV,				// 뭐 대충 나머지들 (delta time, 등등)
	//BONE,
	//ANIMATION_FIRST,
	//ANIMATION_SECOND,				// 늘릴 필요 없었다.
	ROOT_SIGNATURE_IDX_MAX
};

// for multiple render targets
enum class MATERIAL_TYPES {
	ALBEDO = 0,
	ROUGHNESS,
	METALIC,
	SPECULAR,
	NORMAL,
	MATERIAL_END
};

// pre loaded shaders, lighting shader, shadowmapping shader, ...
enum class PRE_LOADED_MATERIALS {
	LIGHTING = 0,
	SHADOWMAPPING,
	OUTLINE,
	PARTICLE,

	BLIT,
#ifdef _DEBUG
	FOR_DEBUG,
#endif // DEBUG
	MATERIAL_END
};

enum class ANIMATION_STATE {
	WAITEND = 0,
	IDLE,
	WALK,
	RUN,
	BLENDED_MOVING_STATE,
	FORWARD_WALK,
	FORWARD_RUN,
	BACKWARD_WALK,
	BACKWARD_RUN,
	LEFT_WALK_STRAFE,
	LEFT_RUN_STRAFE,
	RIGHT_WALK_STRAFE,
	RIGHT_RUN_STRAFE,
	JUMP_START,
	JUMP_ING,
	JUMP_LAND,
	GETTING_HIT,
	GETUP,
	DANCE,
	SIT_START,
	SIT_LOOP,
	SIT_END,		// stand up
	ATTACK,
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
	SHIFT,
	SPACE_BAR,
	INTERACTION,

	// to server
	CTRL,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	ARROW_UP,
	ARROW_DOWN,
	ARROW_LEFT,
	ARROW_RIGHT,
	NEXT,
	PREVIOUS,
	F1,
	F2,
	F3,
	F4,
	F5,
	GAME_INPUT_END
};

#define SEND_SERVER_START static_cast<int>(GAME_INPUT::CTRL)

enum class PARTICLE_TYPES {
	SPARK = 0,
	WATER,
	PARTICLE_TYPE_END
};

ANIMATION_STATE ConvertStringToAnimationState(const std::string& str);
std::string ConvertAnimationStateToString(ANIMATION_STATE anim);
int ConvertGameInputEnumToKeyIntValue(GAME_INPUT gameInput);
PRE_LOADED_MATERIALS ConvertStringToMaterial(const std::string& str);
std::string ConvertMaterialToString(PRE_LOADED_MATERIALS preLaodMat);
PARTICLE_TYPES ConvertStringToParticleType(const std::string& str);
std::string ConvertParticleTypeToString(PARTICLE_TYPES type);

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
	SKINNED,
	BILLBOARD_POSITION_ONLY
};

enum class RESOURCE_TYPES {
	SHADER = 0,
	VERTEX,
	OBJECT,
};

enum class ANIM_ROOTCONST {
	SPACE_BLEND_WEIGHTS = 0,
	FRAMES = 4,
	INDICES = 8,
	PLAYTIME = 12,
	MODE,
	BEFORE_ANIM_BLEND_WEIGHT,
	BONE_INDEX,
	GO_AFFECT_UPPER = 14,
	AFFECT_INDEX = 15,
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
	DEPTH = 10,

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
