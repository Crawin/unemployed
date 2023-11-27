#include <fbxsdk.h>
#include <iostream>
#include <vector>

// 함수 선언
void ProcessNode(fbxsdk::FbxNode* pNode, std::vector<fbxsdk::FbxMesh*>& meshes);

int main() 
{
	// FBX SDK 초기화
	fbxsdk::FbxManager* pManager = fbxsdk::FbxManager::Create();
	fbxsdk::FbxIOSettings* pIOSettings = fbxsdk::FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(pIOSettings);

	// FBX 파일 로드
	const char* filename = "your_model.fbx";
	fbxsdk::FbxImporter* pImporter = fbxsdk::FbxImporter::Create(pManager, "");
	if (!pImporter->Initialize(filename, -1, pManager->GetIOSettings())) {
		std::cerr << "Failed to initialize the FBX importer." << std::endl;
		return -1;
	}
	fbxsdk::FbxScene* pScene = fbxsdk::FbxScene::Create(pManager, "MyScene");

	pImporter->Import(pScene);
	pImporter->Destroy();

	// 루트 노드부터 시작하여 메시 추출
	std::vector<fbxsdk::FbxMesh*> meshes;
	fbxsdk::FbxNode* pRootNode = pScene->GetRootNode();
	if (pRootNode) {
		for (int i = 0; i < pRootNode->GetChildCount(); ++i) {
			ProcessNode(pRootNode->GetChild(i), meshes);
		}
	}

	// 추출된 메시 정보 처리
	for (auto pMesh : meshes) {
		// 여기에서 메시 정보를 처리할 수 있습니다.
		// 메시의 이름: pMesh->GetName()
		std::string name (pMesh->GetName());
		// 메시의 바운딩 박스: pMesh->GetNode()->GetBoundingBoxMin(), pMesh->GetNode()->GetBoundingBoxMax()
		// 메시의 트라이앵글 인덱스: pMesh->GetPolygonVertex(i, j), 여기서 i는 폴리곤 인덱스, j는 정점 인덱스입니다.
	}

	// FBX SDK 정리
	pManager->Destroy();

	return 0;
}

// 노드를 재귀적으로 처리하여 메시 추출
void ProcessNode(fbxsdk::FbxNode* pNode, std::vector<fbxsdk::FbxMesh*>& meshes) {
	if (pNode) {
		fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
		if (pNodeAttribute) {
			if (pNodeAttribute->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh) {
				meshes.push_back(static_cast<fbxsdk::FbxMesh*>(pNodeAttribute));
			}
		}

		// 자식 노드 재귀 호출
		for (int i = 0; i < pNode->GetChildCount(); ++i) {
			ProcessNode(pNode->GetChild(i), meshes);
		}
	}
}
