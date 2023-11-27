#include <iostream>
#include <fstream>
#include <fbxsdk.h>

//#pragma comment (lib, "libfbxsdk.lib")

// �Ͻ� ��� ����

// ���� ���
// 1. fbxsdk�� �ٿ�޴´�.
//		��ũ: https://aps.autodesk.com/developer/overview/fbx-sdk
//		��ġ�Ѵ�. ���� C:\Program Files\Autodesk\ �� ��η� �ٿ��� �Ǿ���
// 2. ê����Ƽ �������� �� �� �� �Ѵ�.
// 3. ����
// 
// 
// ���� ���
// 1. cmd Ų��
// 2. exe�� �ִ� ���Ϸ� cd
// 3. "FbxToOurMesh.exe �����񿢽�����.fbx" ��� ħ
// 4. �ϼ�ǰ�� �����Ѵ� �����񿢽�����.json
// 


// �츮�� �޽� ����
// int						// vertex ����
// float * 3 * vertexNum	// position
// float * 3 * vertexNum	// normal
// float * 3 * vertexNum	// tangent
// float * 2 * vertexNum	// uv
// int						// ����޽��� ����
// ------------------------���� ����޽� ��ŭ �ݺ�------------------------
//		int						// �̸��� ����
//		char * (int��ŭ)			// �̸�
//		float * 3				// boundingbox center
//		float * 3				// boundingboc min
//		float * 3				// boundingboc max
//		int						// submesh index
//



 
void ProcessMesh(FbxNode* pNode, std::ofstream& outputFile)
{
	FbxMesh* pMesh = pNode->GetMesh();
	if (!pMesh) {
		return;
	}

	int vertexCount = pMesh->GetControlPointsCount();
	FbxVector4* vertices = pMesh->GetControlPoints();

	int polygonCount = pMesh->GetPolygonCount();
	int indexCount = 0;

	// JSON ���Ͽ� ����
	
	outputFile << "\t\t{" << std::endl;
	
	// name
	const char* meshName = pNode->GetName();

	// boundingBox
	FbxVector4 min, max, center;
	pNode->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);

	outputFile << "\t\t\"name\": \"" << meshName << "\"," << std::endl;
	outputFile << "\t\t\"min\": [" << min[0] << ", " << min[1] << ", " << min[2] << "]," << std::endl;
	outputFile << "\t\t\"max\": [" << max[0] << ", " << max[1] << ", " << max[2] << "]," << std::endl;
	outputFile << "\t\t\"center\": [" << center[0] << ", " << center[1] << ", " << center[2] << "]," << std::endl;

	// vertex
	outputFile << "\t\t\"vertices\": [" << std::endl;
	for (int i = 0; i < vertexCount; ++i) {
		outputFile << "\t\t\t[" << vertices[i][0] << ", " << vertices[i][1] << ", " << vertices[i][2] << "]";
		if (i < vertexCount - 1) {
			outputFile << ",";
		}
		outputFile << std::endl;
	}
	outputFile << "\t\t]," << std::endl;

	// normal
	FbxLayerElementNormal* pNormalLayer = pMesh->GetLayer(0)->GetNormals();
	if (pNormalLayer) {
		outputFile << "\t\t\"normals\": [" << std::endl;
		for (int i = 0; i < polygonCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				int controlPointIndex = pMesh->GetPolygonVertex(i, j);
				FbxVector4 normal = pNormalLayer->GetDirectArray().GetAt(controlPointIndex);
				outputFile << "\t\t\t[" << normal[0] << ", " << normal[1] << ", " << normal[2] << "]";
				if (i < polygonCount - 1 || j < 2) {
					outputFile << ",";
				}
				outputFile << std::endl;
			}
		}
		outputFile << "\t\t]," << std::endl;
	}

	// tangents
	FbxLayerElementTangent* pTangentLayer = pMesh->GetLayer(0)->GetTangents();
	if (pTangentLayer) {
		outputFile << "\t\t\"tangents\": [" << std::endl;
		for (int i = 0; i < polygonCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				int controlPointIndex = pMesh->GetPolygonVertex(i, j);
				FbxVector4 tan = pTangentLayer->GetDirectArray().GetAt(controlPointIndex);
				outputFile << "\t\t\t[" << tan[0] << ", " << tan[1] << ", " << tan[2] << "]";
				if (i < polygonCount - 1 || j < 2) {
					outputFile << ",";
				}
				outputFile << std::endl;
			}
		}
		outputFile << "\t\t]," << std::endl;
	}

	// uvs
	FbxLayerElementUV* pUVLayer = pMesh->GetLayer(0)->GetUVs();
	if (pUVLayer) {
		outputFile << "\t\t\"uvs\": [" << std::endl;
		for (int i = 0; i < polygonCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				int controlPointIndex = pMesh->GetPolygonVertex(i, j);
				FbxVector2 uv = pUVLayer->GetDirectArray().GetAt(controlPointIndex);
				outputFile << "\t\t[" << uv[0] << ", " << uv[1] << "]";
				if (i < polygonCount - 1 || j < 2) {
					outputFile << ",";
				}
				outputFile << std::endl;
			}
		}
		outputFile << "\t\t]," << std::endl;
	}

	// index
	outputFile << "\t\t\"indices\": [" << std::endl;
	for (int i = 0; i < polygonCount; ++i) {
		outputFile << "      ";
		for (int j = 0; j < 3; ++j) {
			outputFile << pMesh->GetPolygonVertex(i, j) << ", ";
		}
		outputFile << std::endl;
	}
	outputFile << "\t\t]" << std::endl;

	outputFile << "\t}";
}

void ParseFBX(const char* filename, const char* outputJsonFile) 
{
	FbxManager* pManager = FbxManager::Create();
	FbxIOSettings* pIOSettings = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(pIOSettings);

	FbxImporter* pImporter = FbxImporter::Create(pManager, "");

	if (!pImporter->Initialize(filename, -1, pManager->GetIOSettings())) {
		std::cerr << "Failed to initialize FbxImporter!" << std::endl;
		return;
	}

	FbxScene* pScene = FbxScene::Create(pManager, "Scene");
	pImporter->Import(pScene);

	pImporter->Destroy();

	FbxNode* pRootNode = pScene->GetRootNode();
	if (pRootNode) {
		std::string jsonOutputFile = outputJsonFile;
		size_t pos = jsonOutputFile.find_last_of(".");
		if (pos != std::string::npos && pos > 0) {
			// Replace the extension with ".json"
			jsonOutputFile = jsonOutputFile.substr(0, pos) + ".json";
		}
		else {
			// If there is no extension, add ".json" to the end
			jsonOutputFile += ".json";
		}

		// Check if the output file already exists
		std::ifstream existingFile(jsonOutputFile.c_str());
		bool fileExists = existingFile.good();
		existingFile.close();

		// Open the output file for writing
		std::ofstream outputFile(jsonOutputFile, std::ios::out | std::ios::trunc);
		if (!outputFile.is_open()) {
			std::cerr << "Failed to open output file: " << jsonOutputFile << std::endl;
			return;
		}

		outputFile << "{" << std::endl;
		outputFile << "\t\"meshes\": [" << std::endl;

		for (int i = 0; i < pRootNode->GetChildCount(); ++i) {
			ProcessMesh(pRootNode->GetChild(i), outputFile);
			if (i < pRootNode->GetChildCount() - 1) {
				outputFile << ",";
			}
			outputFile << std::endl;
		}

		outputFile << "\t]" << std::endl;
		outputFile << "}" << std::endl;

		outputFile.close();

		if (fileExists) {
			std::cout << "Output file already exists. The existing file was not overwritten." << std::endl;
		}
		else {
			std::cout << "Conversion completed. Output written to: " << jsonOutputFile << std::endl;
		}
	}

	pScene->Destroy();
	pIOSettings->Destroy();
	pManager->Destroy();
}

int main(int argc, char** argv) {
	//if (argc != 2) {
	//	std::cerr << "Usage: " << argv[0] << " <input_fbx_file>" << std::endl;
	//	return 1;
	//}

	const char* fbxFile = "teapot.fbx";
	//const char* fbxFile = argv[1];

	// Generate output JSON file name based on the input FBX file
	std::string jsonOutputFile = fbxFile;
	size_t pos = jsonOutputFile.find_last_of(".");
	if (pos != std::string::npos && pos > 0) {
		// Replace the extension with ".json"
		jsonOutputFile = jsonOutputFile.substr(0, pos) + ".json";
	}
	else {
		// If there is no extension, add ".json" to the end
		jsonOutputFile += ".json";
	}

	ParseFBX(fbxFile, jsonOutputFile.c_str());

	return 0;
}