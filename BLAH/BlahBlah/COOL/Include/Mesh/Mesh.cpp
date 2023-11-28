#include "../framework.h"
#include "../Renderer/Renderer.h"
#include "Mesh.h"

void Mesh::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file)
{
	// �޽� ���� ����
	// 1. �̸� ����					// int
	// 2. �̸�						// char*
	// 3. �ٿ���ڽ�					// float3 x 3 min max center
	// 4. ���ؽ� ����				// int, int*(pos, nor, tan, uv)
	// 5. �ε��� ����				// int int*int
	// 6. ����޽� ����				// int
	// 7. ����޽�(�̸����� �̸� ���ؽ����� ����޽�...����)		// ��ͷ� �İ����
	//

	// 1. �̸� ����
	unsigned int size;
	file.read((char*)&size, sizeof(unsigned int));

	// 2. �̸�
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		file.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. �ٿ���ڽ�	
	XMFLOAT3 min, max;
	file.read((char*)&min, sizeof(XMFLOAT3));
	file.read((char*)&max, sizeof(XMFLOAT3));
	file.read((char*)&m_AABBCenter, sizeof(XMFLOAT3));
	m_AABBExtents = XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);

	// 4-1. ���ؽ� Ÿ��
	unsigned int t;
	file.read((char*)&t, sizeof(unsigned int));

	// 4. ���ؽ� ����
	unsigned int vertexLen = 0;

	// position
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		// vertexLen��ŭ ���ؽ��� ����
		std::vector<XMFLOAT3> position(vertexLen);
		file.read((char*)(&position[0]), sizeof(XMFLOAT3) * vertexLen);
		// todo
		m_PositionBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, position, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	// normal
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT3> normal(vertexLen);
		file.read((char*)(&normal[0]), sizeof(XMFLOAT3) * vertexLen);

		m_NormalBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, normal, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	// tangent
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT3> tangent(vertexLen);
		file.read((char*)(&tangent[0]), sizeof(XMFLOAT3) * vertexLen);

		 m_TangentBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, tangent, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	// uv
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT2> uv(vertexLen);
		file.read((char*)(&uv[0]), sizeof(XMFLOAT2) * vertexLen);

		m_TexCoord0Buffer = Renderer::GetInstance().CreateBufferFromVector(commandList, uv, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	// 5. �ε��� ����				// int int*int
	unsigned int indexNum;
	file.read((char*)&indexNum, sizeof(unsigned int));
	if (indexNum) {
		std::vector<unsigned int> index(indexNum);
		file.read((char*)(&index[0]), sizeof(unsigned int) * indexNum);
		DebugPrint(std::format("name: {}", m_Name));
		DebugPrint(std::format("indexNum: {}, {}", index.size(), index.back()));

		m_IndexBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, index, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	unsigned int childNum;
	file.read((char*)&childNum, sizeof(unsigned int));

	// 6. ����޽� ����
	// 7. ����޽�(���)
	m_Childs.reserve(childNum);
	for (int i = 0; i < childNum; ++i) {
		Mesh newMesh;
		newMesh.BuildMesh(commandList, file);
		m_Childs.push_back(newMesh);
	}
}

bool Mesh::LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName)
{
	std::ifstream meshFile(fileName, std::ios::binary);

	if (meshFile.fail()) {
		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
		return false;
	}

	BuildMesh(commandList, meshFile);
}
