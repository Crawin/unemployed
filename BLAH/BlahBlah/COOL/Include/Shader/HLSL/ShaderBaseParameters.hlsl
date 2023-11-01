


/*
enum ROOT_SIGNATURE_IDX
{
	DESCRIPTOR_HEAP = 0,
	DESCRIPTOR_IDX_CBV,
	CAMERA_DATA_CBV,
	SHADER_DATAS_CBV,
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


#define ALBEDO 0
#define BLABLA 1
// ...

cbuffer Material : register(b0)
{
	int materialIndex[16];
};

cbuffer CameraInfo : register(b1)
{
	matrix viewMatrix : packoffset(c0);
	matrix projectionMatrix : packoffset(c4);
	// float3 cameraPosition : packoffset(c8); <- 이건 뷰변환 행렬에서 뽑아 쓰자
};


SamplerState samplerWarp : register(s0);
SamplerState samplerClamp : register(s1);
