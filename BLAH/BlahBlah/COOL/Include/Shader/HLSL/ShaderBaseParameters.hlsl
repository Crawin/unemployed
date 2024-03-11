


/*
enum ROOT_SIGNATURE_IDX {
	DESCRIPTOR_HEAP = 0,			// 디스크립터힙
	DESCRIPTOR_IDX_CONSTANT,		// 디스크립터를 사용할 인덱스 16개의 int
	CAMERA_DATA_CBV,				// 카메라행렬 4x4 x2
	WORLD_MATRIX,
	SHADER_DATAS_CBV,				// 뭐 대충 나머지들 (delta time, 등등)
	ROOT_SIGNATURE_IDX_MAX
};
*/

//  ...ROOT_SIGNATURE_IDX enum 참고
// idx 0: descriptor table, descriptor table
// idx 1: index of descriptor table, root constants
// idx 2: camera matrix (view, projection(ortho)),
// idx 3: etc (delta time, 
Texture2D Tex2DList[] : register(t0, space0);
TextureCube TexCubeList[] : register(t0, space1);
ByteAddressBuffer RawDataList[] : register(t0, space2);
Buffer<uint> BufferUintList[] : register(t0, space3);
Buffer<float> BufferFloatList[] : register(t0, space4);



// check Enums.h

// for post processing
#define ALBEDO 0
#define ROUGHNESS 1
#define METALIC	2
#define SPECULAR 3
#define NORMALW 4
#define POSITIONW 5
#define MRT_END 6
// ...

cbuffer Material : register(b0)
{
	uint materialIndex[16];
};

cbuffer CameraInfo : register(b1)
{
	matrix viewMatrix;
	matrix projectionMatrix;
	float3 cameraPosition;
};

cbuffer WorldMatrix : register(b2)
{
	matrix localMatrix;
};

cbuffer ShaderVar : register(b3)
{
	float4x4 temp;
};

SamplerState samplerWarp : register(s0);
SamplerState samplerClamp : register(s1);
