#include <iostream>
#include <format>
#include <fstream>
#include <fbxsdk.h>

#define BINARY

std::fstream outputFile;

// �츮�� �޽� ����
// �̸� ����					// size_t
// �̸�						// char*
// �ٿ���ڽ�				// float3 x 3
// ���ؽ� ����				// int, position, int, normal, int, tangent, int, uv, int, index
// ����޽� ����				// int
// ����޽�(�̸����� �̸� ���ؽ����� ����޽�...����)
//

std::string ChangeExtensionToBin(const std::string& originalFileName);

void WriteToFile(int i);
void WriteToFile(const std::string& str);
void WriteToFile(const FbxVector4& vertex, bool onlyVector3 = true);

void DisplayVertex(FbxMesh* pMesh);
void DisplayMesh(FbxNode* pNode);

int main() 
{
	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	// FBX ���� �ε�
	//const char* filename = "AW101.fbx";
	//const char* filename = "ka27.fbx";
	const char* filename = "satodia.fbx";
	std::string outputFileName = ChangeExtensionToBin(filename);

	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
		std::cout << std::format("Failed to initialize importer with file : {}\n", filename);
		return -1;
	}

	FbxScene* scene = FbxScene::Create(manager, "MyScene");
	importer->Import(scene);
	importer->Destroy();

	std::string fileName = outputFileName;

	// ���̳ʸ��� �� �ȿ����°Ű���?
	outputFile.open(fileName, 
#ifdef BINARY
		std::ios::binary | 
#endif
		std::ios::out);

	// ��Ʈ ������ �����Ͽ� �޽��� ���� ���
	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode) {
		int childCount = rootNode->GetChildCount();

		WriteToFile(rootNode->GetName());

		for (int i = 0; i < childCount; i++) {
			std::cout << std::format("Parent: {}\n", rootNode->GetName());
			DisplayMesh(rootNode->GetChild(i));
		}
	}

	scene->Destroy();
	manager->Destroy();

	return 0;
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


void WriteToFile(const std::string& str)
{
#ifdef BINARY
	int temp = str.length();
	outputFile.write((const char*)(&temp), sizeof(temp));
	outputFile.write(str.c_str(), temp);
#else
	outputFile << str.length() << ", " << str.c_str() << "\n";
#endif
}

void WriteToFile(const FbxVector4& vertex, bool onlyVector3)
{
#ifdef BINARY
	int size = 4;
	if (onlyVector3) size = 3;

	for (int i = 0; i < size; ++i) {
		float temp = vertex[i];
		outputFile.write((const char*)(&temp), sizeof(temp));
	}

#else
	if (onlyVector3)	outputFile << vertex[0]  << ", "<< vertex[1]  << ", "<< vertex[2] << "\n";
	else				outputFile << vertex[0]  << ", "<< vertex[1]  << ", "<< vertex[2] << ", " << vertex[3] << "\n";
#endif
}

void WriteToFile(int i)
{
#ifdef BINARY
	outputFile.write((const char*)(&i), sizeof(i));
#else
	outputFile << i << "\n";
#endif
}



void DisplayVertex(FbxMesh* pMesh)
{
	static int vtxTotal = 0;

	int vertexCount = pMesh->GetControlPointsCount();
	FbxVector4* controlPoints = pMesh->GetControlPoints();

	vtxTotal += vertexCount;

	std::cout << std::format("vertex: {}, total {}\n", vertexCount, vtxTotal);

	// position
	WriteToFile(vertexCount);
	for (int i = 0; i < vertexCount; i++) {
		FbxVector4 vertex = controlPoints[i];
		WriteToFile(vertex);
	}

	// normal
	FbxLayerElementNormal* pNormalLayer = pMesh->GetLayer(0)->GetNormals();
	if (pNormalLayer) {

	}

	// tangent
	FbxLayerElementTangent* pTangentLayer = pMesh->GetLayer(0)->GetTangents();
	if (pTangentLayer) {

	}
}

void DisplayMesh(FbxNode* pNode)
{
	FbxMesh* mesh = pNode->GetMesh();
	if (mesh) {
		std::string nodeName = pNode->GetName();
		std::cout << std::format("Mesh: {}\n", nodeName);

		// bounding box
		FbxVector4 min, max, center;
		pNode->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);

		WriteToFile(min);
		WriteToFile(max);
		WriteToFile(center);
		outputFile << "\n";

		WriteToFile(nodeName);
		DisplayVertex(mesh);
		printf("\n");
	}

	int childCount = pNode->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		std::cout << std::format("Parent: {}\n", pNode->GetName());
		DisplayMesh(pNode->GetChild(i));
	}
}