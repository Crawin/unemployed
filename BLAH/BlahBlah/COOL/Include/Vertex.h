#pragma once
#define INTERLEAVED_VERTEX

//vtx info 0
// None

// vtx info 1
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uv;
};

// vtx info 2
struct SkinnedVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uv;
	XMUINT4 blendingIndex;
	XMFLOAT4 blendingFactor;
};