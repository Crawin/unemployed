#include <fbxsdk.h>
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> g_Vector;


void SetBoneIndexSet(FbxNode* node) {
	std::cout << "IDX: " << g_Vector.size() << ", Skeleton node name: " << node->GetName() << std::endl;

	g_Vector.push_back(node->GetName());

	// DFS�� Ž���Ѵ�.
	for (int i = 0; i < node->GetChildCount(); ++i) {
		SetBoneIndexSet(node->GetChild(i));
	}
}


void ExtractSkinningData(FbxMesh* mesh)
{
	int clusterCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

	for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(clusterIndex, FbxDeformer::eSkin));

		int skinClusterCount = skin->GetClusterCount();
		std::cout << "Cluster Count: " << skinClusterCount << std::endl;
		for (int j = 0; j < skinClusterCount; ++j) {
			FbxCluster* cluster = skin->GetCluster(j);
			FbxNode* boneNode = cluster->GetLink();

			if (!boneNode)
				continue;

			int realIdx = -1;
			for (int i = 0; i < g_Vector.size(); ++i) {
				if (g_Vector[i] == boneNode->GetName()) {
					realIdx = i;
					continue;
				}
			}

			if (realIdx < 0) {
				std::cout << "ERROR~!" << std::endl;
				exit(1);
			}

			std::cout << "IDX: " << realIdx << ", " << "Name: " << boneNode->GetName() << std::endl;

			FbxAMatrix transformMatrix;
			cluster->GetTransformMatrix(transformMatrix);
		}
	}
}


void TraverseNode(FbxNode* node)//  std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
	// ��忡 �޽ð� �ִ��� Ȯ��
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0) {
			return ExtractSkinningData(mesh);// , ((SkinnedMesh*)myMesh)->vertices, node->GetNodeAttribute());
		}
	}

	// �ڽ� ���� ��� ȣ��
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		TraverseNode(node->GetChild(i));

	}

	return;
}


void PrintName(FbxNode* node)
{
	std::cout << "name: " << node->GetName() << std::endl;

	for (int i = 0; i < node->GetChildCount(); ++i) {
		PrintName(node->GetChild(i));
	}
}

int main(int argc, char* argv[])
{
	//for (int i = 1; i < argc; ++i)
	{
		// FBX SDK �ʱ�ȭ
		FbxManager* fbxManager = FbxManager::Create();
		FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ios);

		// FBX ���� �ε�
		FbxImporter* importer = FbxImporter::Create(fbxManager, "");

		const char* fileName = "dia_test_rigged.fbx";
		//const char* fileName = argv[i];

		if (!importer->Initialize(fileName, -1, fbxManager->GetIOSettings()))
		{
			// �ε� ���� ó��
			std::cout << "failed to load fbx!!" << std::endl;
			std::cout << "file name: " << fileName << std::endl;
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
			rootNode = rootNode->GetChild(0);
			PrintName(rootNode);

			bool isSkined = false;
			// find skeleton first
			for (int i = 0; i < rootNode->GetChildCount(); ++i) {
				FbxNode* child = rootNode->GetChild(i);
				FbxNodeAttribute* attrib = child->GetNodeAttribute();
				if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
					isSkined = true;
					SetBoneIndexSet(child);
				}
			}
			 
			
			//for (int i = 0; i < rootNode->GetChildCount(); ++i) 
			{
				TraverseNode(rootNode->GetChild(0));

//				std::string outputFileName = "asedf";
//
//				std::fstream outputFile(outputFileName,
//#ifdef BINARY
//					std::ios::binary |
//#endif
//					std::ios::out);
//
//				ExtractToFIle(mesh, outputFile);

				// ���� �۾�
				fbxManager->Destroy();
			}
		}

	}
	return 0;
}








//
//// ��忡�� ��Ʈ �������� ��ȯ ����� ����ϴ� �Լ�
//FbxAMatrix ComputeToRootMatrix(FbxNode* node) {
//    FbxAMatrix toRootMatrix;
//
//    // ��Ʈ ��忡 ������ ������ �� �θ� ����� ��ȯ ����� ����
//    while (node) {
//        toRootMatrix = node->EvaluateGlobalTransform() * toRootMatrix;
//        node = node->GetParent();
//    }
//
//    return toRootMatrix;
//}
//
//void PrintBoneMatrix(FbxMesh* mesh) {
//    int clusterCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
//
//    for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
//        FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(clusterIndex, FbxDeformer::eSkin));
//
//        int clusterCount = skin->GetClusterCount();
//        for (int j = 0; j < clusterCount; ++j) {
//            FbxCluster* cluster = skin->GetCluster(j);
//            FbxNode* boneNode = cluster->GetLink();
//
//            if (!boneNode)
//                continue;
//
//            // ���� ��ȯ ��� ���
//            FbxAMatrix transformMatrix;
//            cluster->GetTransformMatrix(transformMatrix);
//            FbxString boneName = boneNode->GetName();
//            std::cout << "Bone name: " << boneName.Buffer() << std::endl;
//            std::cout << "Transform matrix:" << std::endl;
//            for (int row = 0; row < 4; ++row) {
//                for (int col = 0; col < 4; ++col) {
//                    std::cout << transformMatrix.Get(row, col) << " ";
//                }
//                std::cout << std::endl;
//            }
//            std::cout << std::endl;
//
//            int* indices = cluster->GetControlPointIndices();
//            double* weights = cluster->GetControlPointWeights();
//            int indexCount = cluster->GetControlPointIndicesCount();
//        }
//    }
//}
//
//void PrintSkeletonNodeInfo(FbxNode* node) {
//    if (!node)
//        return;
//
//    FbxNodeAttribute* attribute = node->GetNodeAttribute();
//    if (!attribute || attribute->GetAttributeType() != FbxNodeAttribute::eSkeleton)
//        return;
//
//    // ��� �̸� ���
//    FbxString nodeName = node->GetName();
//    std::cout << "Node name: " << nodeName.Buffer() << std::endl;
//
//    // ��� �Ӽ� ���
//    std::cout << "Node attribute type: Skeleton" << std::endl;
//
//    // Ŭ������ ���� ���
//    FbxNodeAttribute* attribute = node->GetNodeAttribute();
//
//    int clusterCount = node->GetDeformerCount(FbxDeformer::eSkin);
//    std::cout << "Cluster count: " << clusterCount << std::endl;
//
//    for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
//        std::cout << "Cluster index: " << clusterIndex << std::endl;
//
//        FbxSkin* skinDeformer = static_cast<FbxSkin*>(node->GetDeformer(clusterIndex, FbxDeformer::eSkin));
//        if (!skinDeformer)
//            continue;
//
//        int clusterCount = skinDeformer->GetClusterCount();
//        for (int j = 0; j < clusterCount; ++j) {
//            FbxCluster* cluster = skinDeformer->GetCluster(j);
//            FbxNode* boneNode = cluster->GetLink();
//            if (boneNode) {
//                // Ŭ�����Ϳ� ����� ���� ���� ���
//                PrintBoneTransformMatrix(boneNode);
//            }
//        }
//    }
//
//    // ����� ��ȯ ��� ���
//    FbxAMatrix transformMatrix = node->EvaluateGlobalTransform();
//    std::cout << "Transform matrix:" << std::endl;
//    for (int row = 0; row < 4; ++row) {
//        for (int col = 0; col < 4; ++col) {
//            std::cout << transformMatrix.Get(row, col) << " ";
//        }
//        std::cout << std::endl;
//    }
//
//    // �θ� ��� ���
//    FbxNode* parentNode = node->GetParent();
//    if (parentNode) {
//        FbxString parentName = parentNode->GetName();
//        std::cout << "Parent node: " << parentName.Buffer() << std::endl;
//    }
//    else {
//        std::cout << "Parent node: None" << std::endl;
//    }
//
//    // �ڽ� ��� ���
//    int childCount = node->GetChildCount();
//    if (childCount > 0) {
//        std::cout << "Child nodes:" << std::endl;
//        for (int i = 0; i < childCount; ++i) {
//            FbxNode* childNode = node->GetChild(i);
//            FbxString childName = childNode->GetName();
//            std::cout << "  - " << childName.Buffer() << std::endl;
//        }
//    }
//    else {
//        std::cout << "Child nodes: None" << std::endl;
//    }
//
//    std::cout << std::endl;
//}
//
//
//
//
//int main() {
//    // FBX SDK �ʱ�ȭ
//    FbxManager* sdkManager = FbxManager::Create();
//    FbxScene* scene = FbxScene::Create(sdkManager, "myScene");
//
//    // FBX ���� �ε�
//    FbxImporter* importer = FbxImporter::Create(sdkManager, "");
//    importer->Initialize("dia_rigged.fbx", -1, sdkManager->GetIOSettings());
//    importer->Import(scene);
//    importer->Destroy();
//
//    // ��Ʈ ��� ��������
//    FbxNode* rootNode = scene->GetRootNode();
//
//    // ��ȯ ��� ���
//    if (rootNode) {
//
//        for (int i = 0; i < rootNode->GetChildCount(); ++i) {
//            FbxNode* child = rootNode->GetChild(i);
//
//            PrintSkeletonNodeInfo(child);
//
//            //FbxNodeAttribute* attrib = child->GetNodeAttribute();
//            //if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
//            //    PrintBoneMatrix(child->GetMesh());
//            //}
//        }
//    }
//
//    // �޸� ����
//    scene->Destroy();
//    sdkManager->Destroy();
//
//    return 0;
//}
