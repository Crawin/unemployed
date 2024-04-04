#include "Common.h"
#include "Mesh.h"


void Mesh::LoadMeshData(std::ifstream& meshFile)
{
	// �޽� ���� ����
// 1. �̸� ����					// int
// 2. �̸�						// char*
// 3. �ٿ���ڽ�					// float3 x 3
// 4. �θ� ��� ��ȯ ���			// float4x4
// 5. ���ؽ� Ÿ��				// int
// 6. ���ؽ� ����				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 7. �ε��� ����				// int int*int
// 8. ����޽� ����				// int
// 9. ����޽�(�̸����� �̸� ���ؽ����� ����޽�...����)		// ��ͷ� �İ�����
//

// 1. �̸� ����
	unsigned int size;
	meshFile.read((char*)&size, sizeof(unsigned int));

	// 2. �̸�
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		meshFile.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. �ٿ���ڽ�	
	DirectX::XMFLOAT3 min, max;
	meshFile.read((char*)&min, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&max, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&m_AABBCenter, sizeof(DirectX::XMFLOAT3));
	m_AABBExtents = DirectX::XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);

	// 4. �θ� ��� ��ȯ ���		// float4x4
	meshFile.read((char*)&m_LocalTransform, sizeof(DirectX::XMFLOAT4X4));

	// 5. ���ؽ� Ÿ�� // ��� �� �� �ϴ�. ex) ��Ų�޽� vs �Ϲݸ޽�
	unsigned int vtxType;
	meshFile.read((char*)&vtxType, sizeof(unsigned int));

	// 6. ���ؽ� ����
	unsigned int vertexLen = 0;
	meshFile.read((char*)&vertexLen, sizeof(unsigned int));

	if (vertexLen > 0) {

		std::vector<Vertex> vertex(vertexLen);
		meshFile.read((char*)(&vertex[0]), sizeof(Vertex) * vertexLen);

		m_VertexNum = vertexLen;
	}

	// 8. ����޽� ����
	unsigned int childNum;
	meshFile.read((char*)&childNum, sizeof(unsigned int));
	m_Childs.reserve(childNum);

	// 9. ����޽�(���)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh newMesh;
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