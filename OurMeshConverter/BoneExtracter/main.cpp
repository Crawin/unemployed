#include <fbxsdk.h>
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> g_Vector;


void SetBoneIndexSet(FbxNode* node) {
	std::cout << "IDX: " << g_Vector.size() << ", Skeleton node name: " << node->GetName() << std::endl;

	g_Vector.push_back(node->GetName());

	// DFS로 탐색한다.
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
	// 노드에 메시가 있는지 확인
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0) {
			return ExtractSkinningData(mesh);// , ((SkinnedMesh*)myMesh)->vertices, node->GetNodeAttribute());
		}
	}

	// 자식 노드로 재귀 호출
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
		// FBX SDK 초기화
		FbxManager* fbxManager = FbxManager::Create();
		FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ios);

		// FBX 파일 로드
		FbxImporter* importer = FbxImporter::Create(fbxManager, "");

		const char* fileName = "dia_test_rigged.fbx";
		//const char* fileName = argv[i];

		if (!importer->Initialize(fileName, -1, fbxManager->GetIOSettings()))
		{
			// 로딩 실패 처리
			std::cout << "failed to load fbx!!" << std::endl;
			std::cout << "file name: " << fileName << std::endl;
			return -1;
		}

		// 씬 생성
		FbxScene* scene = FbxScene::Create(fbxManager, "MyScene");
		importer->Import(scene);
		importer->Destroy();

		// 노드 탐색 및 정점 및 인덱스 추출
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

				// 정리 작업
				fbxManager->Destroy();
			}
		}

	}
	return 0;
}








//
//// 노드에서 루트 노드까지의 변환 행렬을 계산하는 함수
//FbxAMatrix ComputeToRootMatrix(FbxNode* node) {
//    FbxAMatrix toRootMatrix;
//
//    // 루트 노드에 도달할 때까지 각 부모 노드의 변환 행렬을 곱함
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
//            // 본의 변환 행렬 출력
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
//    // 노드 이름 출력
//    FbxString nodeName = node->GetName();
//    std::cout << "Node name: " << nodeName.Buffer() << std::endl;
//
//    // 노드 속성 출력
//    std::cout << "Node attribute type: Skeleton" << std::endl;
//
//    // 클러스터 개수 출력
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
//                // 클러스터에 연결된 뼈의 정보 출력
//                PrintBoneTransformMatrix(boneNode);
//            }
//        }
//    }
//
//    // 노드의 변환 행렬 출력
//    FbxAMatrix transformMatrix = node->EvaluateGlobalTransform();
//    std::cout << "Transform matrix:" << std::endl;
//    for (int row = 0; row < 4; ++row) {
//        for (int col = 0; col < 4; ++col) {
//            std::cout << transformMatrix.Get(row, col) << " ";
//        }
//        std::cout << std::endl;
//    }
//
//    // 부모 노드 출력
//    FbxNode* parentNode = node->GetParent();
//    if (parentNode) {
//        FbxString parentName = parentNode->GetName();
//        std::cout << "Parent node: " << parentName.Buffer() << std::endl;
//    }
//    else {
//        std::cout << "Parent node: None" << std::endl;
//    }
//
//    // 자식 노드 출력
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
//    // FBX SDK 초기화
//    FbxManager* sdkManager = FbxManager::Create();
//    FbxScene* scene = FbxScene::Create(sdkManager, "myScene");
//
//    // FBX 파일 로드
//    FbxImporter* importer = FbxImporter::Create(sdkManager, "");
//    importer->Initialize("dia_rigged.fbx", -1, sdkManager->GetIOSettings());
//    importer->Import(scene);
//    importer->Destroy();
//
//    // 루트 노드 가져오기
//    FbxNode* rootNode = scene->GetRootNode();
//
//    // 변환 행렬 출력
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
//    // 메모리 정리
//    scene->Destroy();
//    sdkManager->Destroy();
//
//    return 0;
//}
