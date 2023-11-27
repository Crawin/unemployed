#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <fstream>
#include <fbxsdk.h>
#include <DirectXMath.h>

#define BINARY

// �츮�� �޽� ����
// �̸� ����					// int
// �̸�						// char*
// �ٿ���ڽ�				// float3 x 3
// ���ؽ� ����				// int, int*(pos, nor, tan, uv)
// �ε��� ����				// int int*int
// ����޽� ����				// int
// ����޽�(�̸����� �̸� ���ؽ����� ����޽�...����)
//

using namespace DirectX;

// DirectX 12���� ����� ���� ����ü
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uv;
};

struct Mesh {
	std::string name;

	XMFLOAT3 min;
	XMFLOAT3 max;
	XMFLOAT3 center;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	std::vector<Mesh> subMeshes;
};

void TraverseNode(FbxNode* node, Mesh& myMeshData);//std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
//void TraverseNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
void ExtractVertices(FbxMesh* mesh, std::vector<Vertex>& vertices);
void ExtractIndices(FbxMesh* mesh, std::vector<uint16_t>& indices);

void PrintMeshHierachy(const Mesh& mesh);
void ExtractToFIle(const Mesh& mesh, std::fstream& out);

std::string ChangeExtensionToBin(const std::string& originalFileName);

void WriteToFile(int i, std::fstream& out);
void WriteToFile(const std::string& str, std::fstream& out);
void WriteToFile(const XMFLOAT3& float3, std::fstream& out);
void WriteToFile(const XMFLOAT2& float3, std::fstream& out);
void WriteToFile(const Vertex& vtx, std::fstream& out);

//template<class T>
//void WriteToFile(const std::vector<T>& vtxData, std::fstream& out)
//{
//	// int
//	WriteToFile(vtxData.size(), out);
//
//	// data
//	for (const T& data : vtxData) {
//		WriteToFile(data, out);
//	}
//
//}


int main()
{
	// FBX SDK �ʱ�ȭ
	FbxManager* fbxManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ios);

	// FBX ���� �ε�
	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

	const char* fileName = "satodia.fbx";
	std::string outputFileName = ChangeExtensionToBin(fileName);

	if (!importer->Initialize(fileName, -1, fbxManager->GetIOSettings()))
	{
		// �ε� ���� ó��
		std::cout << "failed to load fbx!!" << std::endl;
		return -1;
	}

	// �� ����
	FbxScene* scene = FbxScene::Create(fbxManager, "MyScene");
	importer->Import(scene);
	importer->Destroy();

	// ��� Ž�� �� ���� �� �ε��� ����
	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode)
	{
		Mesh mesh;

		//// ������ �ε����� ���� ����
		//std::vector<Vertex> vertices;
		//std::vector<uint16_t> indices;

		// ��������� ��� Ž��
		TraverseNode(rootNode, mesh);

		// DirectX 12���� ����� �������� ��ȯ�� ������ ���
		// ...

		//PrintMeshHierachy(mesh);

		std::fstream outputFile(outputFileName,
#ifdef BINARY
			std::ios::binary |
#endif
			std::ios::out);

		ExtractToFIle(mesh, outputFile);

		// ���� �۾�
		fbxManager->Destroy();
	}

	return 0;
}

// ��������� ��带 Ž���Ͽ� ������ �ε����� �����ϴ� �Լ�
/*
void TraverseNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
	// ��忡 �޽ð� �ִ��� Ȯ��
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		// ���� ����
		ExtractVertices(mesh, vertices);

		// �ε��� ����
		ExtractIndices(mesh, indices);
	}

	// �ڽ� ���� ��� ȣ��
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		TraverseNode(node->GetChild(i), vertices, indices);
	}
}
*/

// ��������� ��带 Ž���Ͽ� ������ �ε����� �����ϴ� �Լ�
void TraverseNode(FbxNode* node, Mesh& myMesh)//  std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
	// ��忡 �޽ð� �ִ��� Ȯ��
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		FbxVector4 min, max, center;
		node->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);

		myMesh.min = XMFLOAT3(static_cast<float>(min[0]), static_cast<float>(min[1]), static_cast<float>(min[2]));
		myMesh.max = XMFLOAT3(static_cast<float>(max[0]), static_cast<float>(max[1]), static_cast<float>(max[2]));
		myMesh.center = XMFLOAT3(static_cast<float>(center[0]), static_cast<float>(center[1]), static_cast<float>(center[2]));

		// �̸�
		myMesh.name = node->GetName();

		// ���� ����
		ExtractVertices(mesh, myMesh.vertices);

		// �ε��� ����
		ExtractIndices(mesh, myMesh.indices);
	}

	// �ڽ� ���� ��� ȣ��
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		Mesh newMesh;
		TraverseNode(node->GetChild(i), newMesh);

		myMesh.subMeshes.push_back(newMesh);
	}
}

// FBX �޽ÿ��� ������ �����ϴ� �Լ�
void ExtractVertices(FbxMesh* mesh, std::vector<Vertex>& vertices)
{
	// ������ �� ��������
	int vertexCount = mesh->GetControlPointsCount();

	// ���� ������ ����
	FbxVector4* controlPoints = mesh->GetControlPoints();

	// ������ Vertex ����ü�� ��ȯ�Ͽ� ���Ϳ� �߰�
	for (int i = 0; i < vertexCount; ++i)
	{
		FbxVector4 position = controlPoints[i];

		// DirectX 12���� ����� �������� ��ȯ�Ͽ� ���Ϳ� �߰�
		Vertex vertex;
		vertex.position = XMFLOAT3(static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]));

		// ���� ������ �ִ� ��쿡�� ����
		if (mesh->GetElementNormalCount() > 0)
		{
			FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
			FbxVector4 normal;
			normal = normalElement->GetDirectArray().GetAt(i);
			vertex.normal = XMFLOAT3(static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]));
		}

		// ź��Ʈ ������ �ִ� ��쿡�� ����
		if (mesh->GetElementTangentCount() > 0)
		{
			FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();
			FbxVector4 tangent;
			tangent = tangentElement->GetDirectArray().GetAt(i);
			vertex.tangent = XMFLOAT3(static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), static_cast<float>(tangent[2]));
		}
		else if (mesh->GetElementBinormalCount() > 0)
		{
			FbxGeometryElementBinormal* biNormalElement = mesh->GetElementBinormal();
			FbxVector4 biNormal;
			biNormal = biNormalElement->GetDirectArray().GetAt(i);

			XMFLOAT3 biN = XMFLOAT3(static_cast<float>(biNormal[0]), static_cast<float>(biNormal[1]), static_cast<float>(biNormal[2]));;
			XMFLOAT3 nor = vertex.normal;

			XMVECTOR vectorBiNormal = XMLoadFloat3(&biN);
			XMVECTOR vectorNormal = XMLoadFloat3(&nor);
			XMVECTOR vectorTangent = XMVector3Cross(vectorBiNormal, vectorNormal);

			static bool neverHit = true;
			if (neverHit) {
				std::cout << "Hit Binormal!!! ����!!!!!!!" << std::endl;
				neverHit = false;
			}

			// ����!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			XMFLOAT3 tangent;
			XMStoreFloat3(&tangent, vectorTangent);
			vertex.tangent = tangent;
		}

		// �ؽ�ó ��ǥ ������ �ִ� ��쿡�� ����
		if (mesh->GetElementUVCount() > 0)
		{
			FbxGeometryElementUV* uvElement = mesh->GetElementUV();
			FbxVector2 uv;
			uv = uvElement->GetDirectArray().GetAt(i);
			vertex.uv = XMFLOAT2(static_cast<float>(uv[0]), static_cast<float>(uv[1]));
		}

		vertices.push_back(vertex);
	}
}

// FBX �޽ÿ��� �ε����� �����ϴ� �Լ�
void ExtractIndices(FbxMesh* mesh, std::vector<uint16_t>& indices)
{
	// �޽��� ������ �� ��������
	int polygonCount = mesh->GetPolygonCount();

	// �� �����︶�� �ε��� ����
	for (int i = 0; i < polygonCount; ++i)
	{
		// �������� ���ؽ� �ε��� �� ��������
		int polygonSize = mesh->GetPolygonSize(i);

		// �������� �� ���ؽ� �ε����� �����Ͽ� ���Ϳ� �߰�
		for (int j = 0; j < polygonSize; ++j)
		{
			int vertexIndex = mesh->GetPolygonVertex(i, j);

			// DirectX 12���� uint16_t ������ ����ϹǷ� ������ ��ȯ�� ����
			indices.push_back(static_cast<uint16_t>(vertexIndex));
		}
	}
}

// �׳� ����ϴ� �Լ�
void PrintMeshHierachy(const Mesh& mesh)
{
	std::cout << std::format("name: {}\n", mesh.name);
	std::cout << std::format("vertex: {}\n\n", mesh.vertices.size());

	for (const auto& m : mesh.subMeshes) {
		PrintMeshHierachy(m);
	}
}

// ���Ϸ� ����ϴ� �Լ�
void ExtractToFIle(const Mesh& mesh, std::fstream& out)
{
	std::cout << std::format("name: {}\n", mesh.name);
	std::cout << std::format("vertex: {}\n\n", mesh.vertices.size());

	// name
	WriteToFile(mesh.name, out);

	// �ٿ���ڽ�				// float3 x 3
	WriteToFile(mesh.min, out);
	WriteToFile(mesh.max, out);
	WriteToFile(mesh.center, out);

	// ���ؽ� ����				// int, position, normal, tangent, uv, int, index
	WriteToFile(mesh.vertices.size(), out);
	for (const auto& vtx : mesh.vertices) {
		WriteToFile(vtx, out);
	}

	// �ε��� ����
	WriteToFile(mesh.indices.size(), out);
	for (int i : mesh.indices) {
		WriteToFile(i, out);
	}


	// ����޽� ����				// int
	WriteToFile(mesh.subMeshes.size(), out);
	for (const auto& m : mesh.subMeshes) {
		ExtractToFIle(m, out);
	}
}

std::string ChangeExtensionToBin(const std::string& originalFileName)
{
	size_t dotPosition = originalFileName.find_last_of('.');
	if (dotPosition != std::string::npos) {
		// Ȯ���ڰ� �ִ� ���
		std::string fileNameWithoutExtension = originalFileName.substr(0, dotPosition);
#ifdef BINARY
		return fileNameWithoutExtension + ".bin";
#else
		return fileNameWithoutExtension + ".txt";
#endif
	}
	else {
		// Ȯ���ڰ� ���� ���
#ifdef BINARY
		return originalFileName + ".bin";
#else
		return originalFileName + ".txt";
#endif
	}
}

void WriteToFile(int i, std::fstream& out)
{
#ifdef BINARY
	out.write((const char*)(&i), sizeof(i));
#else
	out << i << "\n";
#endif
}

void WriteToFile(const std::string& str, std::fstream& out)
{
#ifdef BINARY
	int temp = str.length();
	out.write((const char*)(&temp), sizeof(temp));
	out.write(str.c_str(), temp);
#else
	out << str.length() << ", " << str.c_str() << "\n";
#endif
}

void WriteToFile(const XMFLOAT3& float3, std::fstream& out)
{
#ifdef BINARY
	out.write((const char*)(&float3), sizeof(XMFLOAT3));
#else
	out << float3.x << ", " << float3.y << ", " << float3.z << "\n";
#endif
}

void WriteToFile(const XMFLOAT2& float2, std::fstream& out)
{
#ifdef BINARY
	out.write((const char*)(&float2), sizeof(XMFLOAT2));
#else
	out << float2.x << ", " << float2.y << "\n";
#endif
}

void WriteToFile(const Vertex& vtx, std::fstream& out)
{
	WriteToFile(vtx.position, out);
	WriteToFile(vtx.normal, out);
	WriteToFile(vtx.tangent, out);
	WriteToFile(vtx.uv, out);
}
