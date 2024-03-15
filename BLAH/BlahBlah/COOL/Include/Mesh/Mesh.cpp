#include "framework.h"
#include "Mesh.h"
#include "Scene/ResourceManager.h"

template<typename VERTEX>
inline void Mesh::LoadVertices(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager, int vtxSize)
{
	std::vector<VERTEX> vertex(vtxSize);
	file.read((char*)(&vertex[0]), sizeof(VERTEX) * vtxSize);

	m_VertexBuffer = manager->CreateBufferFromVector(
		commandList,
		vertex,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		std::format("{}_vertex data", m_Name));

	m_VertexBufferView.BufferLocation = manager->GetVertexDataGPUAddress(m_VertexBuffer);
	m_VertexBufferView.StrideInBytes = sizeof(VERTEX);
	m_VertexBufferView.SizeInBytes = sizeof(VERTEX) * vtxSize;
};

void Mesh::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, const std::string& fileName, ResourceManager* manager)
{
	// 메쉬 파일 구조
	// 1. 이름 길이					// int
	// 2. 이름						// char*
	// 3. 바운딩박스					// float3 x 3
	// 4. 부모 상대 변환 행렬			// float4x4
	// 5. 버텍스 타입				// int
	// 6. 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv, + uint4(bleidng bone) + float4 (blending weight)
	// 7. 인덱스 정보				// int int*int
	// 8. 서브메쉬 개수				// int
	// 9. 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)		// 재귀로 파고들어라
	//

	// 1. 이름 길이
	unsigned int size;
	file.read((char*)&size, sizeof(unsigned int));

	// 2. 이름
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		file.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. 바운딩박스	
	XMFLOAT3 min, max;
	file.read((char*)&min, sizeof(XMFLOAT3));
	file.read((char*)&max, sizeof(XMFLOAT3));
	file.read((char*)&m_AABBCenter, sizeof(XMFLOAT3));
	m_AABBExtents = XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);

	// 4. 부모 상대 변환 행렬		// float4x4
	file.read((char*)&m_LocalTransform, sizeof(XMFLOAT4X4));

	// 5. 버텍스 타입 // 사용 할 듯 하다. ex) 스킨메쉬 vs 일반메쉬
	unsigned int vtxType;
	file.read((char*)&vtxType, sizeof(unsigned int));

	// 6. 버텍스 정보
	unsigned int vertexLen = 0;

#ifdef INTERLEAVED_VERTEX
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {

		switch (static_cast<VERTEX_TYPES>(vtxType)) {
		case VERTEX_TYPES::NO_VERTEX:
			break;

		case VERTEX_TYPES::NORMAL:
			LoadVertices<Vertex>(commandList, file, manager, vertexLen);
			break;

		case VERTEX_TYPES::SKINNED:
			LoadVertices<SkinnedVertex>(commandList, file, manager, vertexLen);
			GetBone(commandList, fileName, manager);
			break;
		}


		m_VertexNum = vertexLen;
	}

#else
	// position
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		// vertexLen만큼 버텍스를 생성
		std::vector<XMFLOAT3> position(vertexLen);
		file.read((char*)(&position[0]), sizeof(XMFLOAT3) * vertexLen);
		// todo
		m_PositionBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, position, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("{}_position", m_Name));

		m_PositionBufferView.BufferLocation = Renderer::GetInstance().GetVertexDataGPUAddress(m_PositionBuffer);
		m_PositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_PositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * vertexLen;

		m_VertexNum = vertexLen;
	}

	// normal
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT3> normal(vertexLen);
		file.read((char*)(&normal[0]), sizeof(XMFLOAT3) * vertexLen);

		m_NormalBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, normal, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("{}_normal", m_Name));

		m_NormalBufferView.BufferLocation = Renderer::GetInstance().GetVertexDataGPUAddress(m_NormalBuffer);
		m_NormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_NormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * vertexLen;
	}

	// tangent
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT3> tangent(vertexLen);
		file.read((char*)(&tangent[0]), sizeof(XMFLOAT3) * vertexLen);

		m_TangentBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, tangent, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("{}_tangent", m_Name));

		m_TangentBufferView.BufferLocation = Renderer::GetInstance().GetVertexDataGPUAddress(m_TangentBuffer);
		m_TangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_TangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * vertexLen;
	}

	// uv
	file.read((char*)&vertexLen, sizeof(unsigned int));
	if (vertexLen > 0) {
		std::vector<XMFLOAT2> uv(vertexLen);
		file.read((char*)(&uv[0]), sizeof(XMFLOAT2) * vertexLen);

		m_TexCoord0Buffer = Renderer::GetInstance().CreateBufferFromVector(commandList, uv, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("{}_uv", m_Name));

		m_TexCoord0BufferView.BufferLocation = Renderer::GetInstance().GetVertexDataGPUAddress(m_TexCoord0Buffer);
		m_TexCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
		m_TexCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * vertexLen;
	}

	// 7. 인덱스 정보				// int int*int
	unsigned int indexNum = 0;
	file.read((char*)&indexNum, sizeof(unsigned int));
	if (indexNum > 0) {
		std::vector<unsigned int> index(indexNum);
		file.read((char*)(&index[0]), sizeof(unsigned int) * indexNum);
		DebugPrint(std::format("name: {}", m_Name));
		DebugPrint(std::format("indexNum: {}, {}", index.size(), index.back()));

		m_IndexBuffer = Renderer::GetInstance().CreateBufferFromVector(commandList, index, D3D12_RESOURCE_STATE_INDEX_BUFFER, std::format("{}_index", m_Name));

		m_IndexBufferView.BufferLocation = Renderer::GetInstance().GetVertexDataGPUAddress(m_IndexBuffer);
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_IndexBufferView.SizeInBytes = sizeof(unsigned int) * indexNum;

		m_IndexNum = indexNum;
	}
#endif

	manager->AddMesh(this);

	// 8. 서브메쉬 개수
	unsigned int childNum;
	file.read((char*)&childNum, sizeof(unsigned int));
	m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh* newMesh = new Mesh;
		newMesh->BuildMesh(commandList, file, fileName, manager);
		m_Childs.push_back(newMesh);
	}
}

int Mesh::GetBone(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName, ResourceManager* manager)
{
	int idx = manager->GetBone(fileName, commandList);
	if (idx != -1) {
		return idx;
	}

	ERROR_QUIT(std::format("No Such Bone File!!, fileName: {}", fileName));
	//exit(1);
}

//bool Mesh::LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName)
//{
//	std::ifstream meshFile(fileName, std::ios::binary);
//
//	if (meshFile.fail()) {
//		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
//		return false;
//	}
//
//	BuildMesh(commandList, meshFile);
//	return true;
//}

void Mesh::Render(ComPtr<ID3D12GraphicsCommandList> commandList, XMFLOAT4X4& parent)
{
	if (m_VertexNum > 0) {
#ifdef INTERLEAVED_VERTEX
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[1] = {
			m_VertexBufferView
		};
#else
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[4] = {
			m_PositionBufferView,
			m_NormalBufferView,
			m_TangentBufferView,
			m_TexCoord0BufferView
		};

		commandList->IASetIndexBuffer(&m_IndexBufferView);

#endif

		m_RootTransform = Matrix4x4::Multiply(m_LocalTransform, parent);

		XMFLOAT4X4 temp = Matrix4x4::Transpose(m_RootTransform);

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::WORLD_MATRIX), 16, &temp, 0);

		commandList->IASetVertexBuffers(0, _countof(vertexBufferViews), vertexBufferViews);

#ifdef INTERLEAVED_VERTEX
		commandList->DrawInstanced(m_VertexNum, 1, 0, 0);
#else
		commandList->DrawIndexedInstanced(m_IndexNum, 1, 0, 0, 0);
#endif
	}

	//for (auto& mesh : m_Childs)
	//	mesh->Render(commandList, m_RootTransform);
}
