#include "IOCP_Common.h"
#include "Mesh.h"

void Mesh::LoadMeshData(std::ifstream& meshFile)
{
	// 메쉬 파일 구조
// 1. 이름 길이					// int
// 2. 이름						// char*
// 3. 바운딩박스					// float3 x 3
// 4. 부모 상대 변환 행렬			// float4x4
// 5. 버텍스 타입				// int
// 6. 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 7. 인덱스 정보				// int int*int
// 8. 서브메쉬 개수				// int
// 9. 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)		// 재귀로 파고들어라
//

// 1. 이름 길이
	unsigned int size;
	meshFile.read((char*)&size, sizeof(unsigned int));

	// 2. 이름
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		meshFile.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. 바운딩박스	
	DirectX::XMFLOAT3 min, max;
	meshFile.read((char*)&min, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&max, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&m_AABBCenter, sizeof(DirectX::XMFLOAT3));
	m_AABBExtents = DirectX::XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);
	m_AABBExtents_Divide = DirectX::XMFLOAT3(m_AABBExtents.x / 2, m_AABBExtents.y / 2, m_AABBExtents.z / 2);
	// 4. 부모 상대 변환 행렬		// float4x4
	meshFile.read((char*)&m_LocalTransform, sizeof(DirectX::XMFLOAT4X4));

	// 5. 버텍스 타입 // 사용 할 듯 하다. ex) 스킨메쉬 vs 일반메쉬
	unsigned int vtxType;
	meshFile.read((char*)&vtxType, sizeof(unsigned int));

	// 6. 버텍스 정보
	unsigned int vertexLen = 0;
	meshFile.read((char*)&vertexLen, sizeof(unsigned int));

	if (vertexLen > 0) {

		std::vector<Vertex> vertex(vertexLen);
		meshFile.read((char*)(&vertex[0]), sizeof(Vertex) * vertexLen);

		m_VertexNum = vertexLen;
	}

	// 8. 서브메쉬 개수
	unsigned int childNum;
	meshFile.read((char*)&childNum, sizeof(unsigned int));
	m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh newMesh;
		DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4(&m_RootTransform);
		DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&m_LocalTransform);
		DirectX::XMStoreFloat4x4(&newMesh.m_RootTransform, DirectX::XMMatrixMultiply(root, local));
		newMesh.LoadMeshData(meshFile);
		m_Childs.push_back(newMesh);
	}
}

DirectX::XMFLOAT3 Mesh::GetCenter()
{
	return m_AABBCenter;
}

DirectX::XMFLOAT3 Mesh::GetExtents()
{
	return m_AABBExtents;
}

bool Mesh::collision(const DirectX::BoundingOrientedBox& player)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	DirectX::BoundingOrientedBox obb(m_AABBCenter, m_AABBExtents_Divide, orient);

	//std::cout << this->m_Name << " Center : (" << obb.Center.x << "," << obb.Center.y << "," << obb.Center.z << "), Extents : ("
	//	<< obb.Extents.x << "," << obb.Extents.y << "," << obb.Extents.z << ")" << std::endl;
	if (player.Intersects(obb))
	{
		return true;
	}
	else
	{
		for (auto& child : m_Childs)
		{
			if (child.collision(player))
				return true;
		}
	}
	return false;
}
