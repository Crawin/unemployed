


/*
enum ROOT_SIGNATURE_IDX {
	DESCRIPTOR_HEAP = 0,			// ��ũ������
	DESCRIPTOR_IDX_CONSTANT,		// ��ũ���͸� ����� �ε��� 16���� int
	CAMERA_DATA_CBV,				// ī�޶���� 4x4 x2
	WORLD_MATRIX,
	SHADER_DATAS_CBV,				// �� ���� �������� (delta time, ���)
	ROOT_SIGNATURE_IDX_MAX
};
*/

//  ...ROOT_SIGNATURE_IDX enum ����
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