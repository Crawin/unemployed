#include <fbxsdk.h>
#include <iostream>
#include <vector>

// �Լ� ����
void ProcessNode(fbxsdk::FbxNode* pNode, std::vector<fbxsdk::FbxMesh*>& meshes);

int main() 
{
	// FBX SDK �ʱ�ȭ
	fbxsdk::FbxManager* pManager = fbxsdk::FbxManager::Create();
	fbxsdk::FbxIOSettings* pIOSettings = fbxsdk::FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(pIOSettings);

	// FBX ���� �ε�
	const char* filename = "your_model.fbx";
	fbxsdk::FbxImporter* pImporter = fbxsdk::FbxImporter::Create(pManager, "");
	if (!pImporter->Initialize(filename, -1, pManager->GetIOSettings())) {
		std::cerr << "Failed to initialize the FBX importer." << std::endl;
		return -1;
	}
	fbxsdk::FbxScene* pScene = fbxsdk::FbxScene::Create(pManager, "MyScene");

	pImporter->Import(pScene);
	pImporter->Destroy();

	// ��Ʈ ������ �����Ͽ� �޽� ����
	std::vector<fbxsdk::FbxMesh*> meshes;
	fbxsdk::FbxNode* pRootNode = pScene->GetRootNode();
	if (pRootNode) {
		for (int i = 0; i < pRootNode->GetChildCount(); ++i) {
			ProcessNode(pRootNode->GetChild(i), meshes);
		}
	}

	// ����� �޽� ���� ó��
	for (auto pMesh : meshes) {
		// ���⿡�� �޽� ������ ó���� �� �ֽ��ϴ�.
		// �޽��� �̸�: pMesh->GetName()
		std::string name (pMesh->GetName());
		// �޽��� �ٿ�� �ڽ�: pMesh->GetNode()->GetBoundingBoxMin(), pMesh->GetNode()->GetBoundingBoxMax()
		// �޽��� Ʈ���̾ޱ� �ε���: pMesh->GetPolygonVertex(i, j), ���⼭ i�� ������ �ε���, j�� ���� �ε����Դϴ�.
	}

	// FBX SDK ����
	pManager->Destroy();

	return 0;
}

// ��带 ��������� ó���Ͽ� �޽� ����
void ProcessNode(fbxsdk::FbxNode* pNode, std::vector<fbxsdk::FbxMesh*>& meshes) {
	if (pNode) {
		fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
		if (pNodeAttribute) {
			if (pNodeAttribute->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh) {
				meshes.push_back(static_cast<fbxsdk::FbxMesh*>(pNodeAttribute));
			}
		}

		// �ڽ� ��� ��� ȣ��
		for (int i = 0; i < pNode->GetChildCount(); ++i) {
			ProcessNode(pNode->GetChild(i), meshes);
		}
	}
}
