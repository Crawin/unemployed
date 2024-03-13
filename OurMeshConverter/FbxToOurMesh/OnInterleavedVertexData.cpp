#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <algorithm>
#include <fstream>
#include <fbxsdk.h>
#include <DirectXMath.h>
#define BINARY

// 우리의 메쉬 순서
// 이름 길이					// int
// 이름						// char*
// 바운딩박스				// float3 x 3
// 부모 상대 변환 행렬		// float4x4
// 버텍스 타입				// int
// 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 인덱스 정보				// int int*int
// 서브메쉬 개수				// int
// 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)
//

#define ALL_IN_ONE
#define MAX_CONTROL_POINT 4
using namespace DirectX;

std::vector<std::string> g_Vector;
std::vector<XMFLOAT4X4> g_BoneMatrixVector;

void SetBoneIndexSet(FbxNode* node) {

	// no bone
	if (g_Vector.end() == std::find(g_Vector.begin(), g_Vector.end(), node->GetName())) {
		std::cout << "IDX: " << g_Vector.size() << "Skeleton node name: " << node->GetName() << std::endl;
		XMFLOAT4X4 trans;
		FbxMatrix fbxTransform = node->EvaluateGlobalTransform();
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				trans.m[i][j] = fbxTransform[i][j];
			}
		}
		// insert data to vector
		g_Vector.push_back(node->GetName());
		g_BoneMatrixVector.push_back(trans);
	}

	// DFS로 탐색한다.
	for (int i = 0; i < node->GetChildCount(); ++i) {
		SetBoneIndexSet(node->GetChild(i));
	}
}

void ConvertToLH(XMFLOAT3& vtx)
{
	vtx.x *= -1;

}

// DirectX 12에서 사용할 정점 구조체
// mesh type 4
struct Vertex
{
	XMFLOAT3 position = { 0,0,0 };
	XMFLOAT3 normal = { 0,0,0 };
	XMFLOAT3 tangent = { 0,0,0 };
	XMFLOAT2 uv = { 0,0 };
};

void WriteToFile(unsigned int i, std::fstream& out);
void WriteToFile(const std::string& str, std::fstream& out);
void WriteToFile(const XMFLOAT3& float3, std::fstream& out);
void WriteToFile(const XMFLOAT2& float3, std::fstream& out);
void WriteToFile(const XMFLOAT4X4& matrix, std::fstream& out);
void WriteToFile(const Vertex& vtx, std::fstream& out);



// 스킨메쉬에 사용할 정점 구조체, 스키닝 정보가 있다.
// mesh type 6
struct SkinnedVertex
{
	XMFLOAT3 position = { 0,0,0 };
	XMFLOAT3 normal = { 0,0,0 };
	XMFLOAT3 tangent = { 0,0,0 };
	XMFLOAT2 uv = { 0,0 };
	XMUINT4 blendingIndex = { 0,0,0,0 };
	XMFLOAT4 blendingFactor = { 0,0,0,0 };
};

template <typename T>
void swap(T& a, T& b) {
	T t = a;
	a = b;
	b = t;
}

struct MeshBase {
	std::string name = "";

	XMFLOAT3 min = { 0,0,0 };
	XMFLOAT3 max = { 0,0,0 };
	XMFLOAT3 center = { 0,0,0 };

	XMFLOAT4X4 localTransform = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, };

	std::vector<MeshBase*> subMeshes;

	virtual void ConvertToLeftHanded() = 0;

	virtual int GetVtxSize() const = 0;

	virtual void WriteVtxInfoAndData(std::fstream& out) const = 0;
};

struct NormalMesh : public MeshBase {
	std::vector<Vertex> vertices;

#ifndef ALL_IN_ONE
	std::vector<uint16_t> indices;
#endif

	virtual void ConvertToLeftHanded()
	{
		// swap x
		for (int i = 0; i < vertices.size(); ++i) {
			ConvertToLH(vertices[i].position);
			ConvertToLH(vertices[i].normal);
			ConvertToLH(vertices[i].tangent);
		}

		// for winding order
		for (int i = 0; i < vertices.size(); i += 3) {
			swap(vertices[i + 1], vertices[i + 2]);
		}

		for (auto& subMesh : subMeshes) subMesh->ConvertToLeftHanded();
	}

	virtual int GetVtxSize() const { return vertices.size(); }

	virtual void WriteVtxInfoAndData(std::fstream& out) const
	{
		WriteToFile(1, out);
		WriteToFile(vertices.size(), out);
		if (vertices.size() > 0)
			out.write((const char*)(&vertices[0]), sizeof(Vertex) * vertices.size());
	}

};

struct SkinnedMesh : public MeshBase {
	std::vector<SkinnedVertex> vertices;

#ifndef ALL_IN_ONE
	std::vector<uint16_t> indices;
#endif
	virtual void ConvertToLeftHanded()
	{
		// todo 더 해야함

		// swap x
		for (auto& vtx : vertices) {
			ConvertToLH(vtx.position);
			ConvertToLH(vtx.normal);
			ConvertToLH(vtx.tangent);
		}

		// for winding order
		for (int i = 0; i < vertices.size(); i += 3) {
			swap(vertices[i + 1], vertices[i + 2]);
		}

		for (auto& subMesh : subMeshes) subMesh->ConvertToLeftHanded();
	}

	virtual int GetVtxSize() const { return vertices.size(); }

	virtual void WriteVtxInfoAndData(std::fstream& out) const
	{
		WriteToFile(2, out);
		WriteToFile(vertices.size(), out);
		if (vertices.size() > 0)
			out.write((const char*)(&vertices[0]), sizeof(SkinnedVertex) * vertices.size());
	}
};


std::string removeExtension(const std::string& filename) {
	size_t lastDotPos = filename.find_last_of('.');
	if (lastDotPos != std::string::npos) { // '.'이 발견되면
		return filename.substr(0, lastDotPos); // '.' 이전의 부분을 반환
	}
	return filename; // '.'이 발견되지 않으면 원래 문자열을 그대로 반환
}

MeshBase* TraverseNode(FbxNode* node);//std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
//void TraverseNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
void ExtractVertices(FbxMesh* mesh, std::vector<Vertex>& vertices, FbxNodeAttribute* attribute);
void ExtractVertices(FbxMesh* mesh, std::vector<SkinnedVertex>& vertices, FbxNodeAttribute* attribute);

void ExtractSkinningData(FbxMesh* mesh, int vertexIndex, SkinnedVertex& vertex);

void ExtractIndices(FbxMesh* mesh, std::vector<uint16_t>& indices);

void PrintMeshHierachy(const MeshBase* mesh);
void ExtractToFIle(const MeshBase* mesh, std::fstream& out);

std::string ChangeExtensionToBin(const std::string& originalFileName);

void FindBone(FbxNode* Node)
{
	FbxNodeAttribute* attrib = Node->GetNodeAttribute();
	if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		// find bone!
		SetBoneIndexSet(Node);
	}

	// n
	for (int i = 0; i < Node->GetChildCount(); ++i) {
		FindBone(Node->GetChild(i));
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

		//const char* fileName = "test_rigging.fbx";
		const char* fileName = "x-35_fbx.fbx";
		//const char* fileName = argv[i];

		//outputFileName = "teapot.bin";

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
			
			//FbxNode* mainRoot = rootNode;// = rootNode->GetChild(0);

			// find skeleton first
			FindBone(rootNode);

			// if bone, export
			if (g_BoneMatrixVector.size() > 0) {
				// export bone file
				std::string boneFileName = "bone_";
				boneFileName += fileName;
				boneFileName += ".bin";

				std::fstream output(boneFileName, std::ios::binary | std::ios::out);

				WriteToFile((int)(g_BoneMatrixVector.size()), output);
				output.write((const char*)(&g_BoneMatrixVector[0]), sizeof(XMFLOAT4X4) * g_BoneMatrixVector.size());
			}


			MeshBase* mesh = new NormalMesh;
			mesh->name = removeExtension(fileName);

			for (int i = 0; i < rootNode->GetChildCount(); ++i)
			{
				FbxNode* child = rootNode->GetChild(i);
				FbxNodeAttribute* attrib = child->GetNodeAttribute();
				if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton) 
					continue;

				// 재귀적으로 노드 탐색
				// RootNode가 따로 있더라
				// 따로따로인 모델이 있을수도 있어서 그런가봄
				// 우리는 부모메쉬가 하나라고 가정

				MeshBase* t = TraverseNode(child);
				if (t != nullptr) mesh->subMeshes.push_back(t);
				
				// DirectX 12에서 사용할 형식으로 변환된 데이터 사용
				// 
				if (scene->GetGlobalSettings().GetAxisSystem().GetCoorSystem() != FbxAxisSystem::eLeftHanded) {
					std::cout << "Convert to Lefthanded!" << std::endl;
					mesh->ConvertToLeftHanded();
				}
			}
			
			std::string outputFileName = ChangeExtensionToBin(fileName);
			std::fstream outputFile(outputFileName, std::ios::binary | std::ios::out);
			ExtractToFIle(mesh, outputFile);

			// 정리 작업
			fbxManager->Destroy();
		}

	}
	return 0;
}

// 재귀적으로 노드를 탐색하여 정점과 인덱스를 추출하는 함수
/*
void TraverseNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
	// 노드에 메시가 있는지 확인
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		// 정점 추출
		ExtractVertices(mesh, vertices);

		// 인덱스 추출
		ExtractIndices(mesh, indices);
	}

	// 자식 노드로 재귀 호출
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		TraverseNode(node->GetChild(i), vertices, indices);
	}
}
*/

// 재귀적으로 노드를 탐색하여 정점과 인덱스를 추출하는 함수
MeshBase* TraverseNode(FbxNode* node)//  std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{

	MeshBase* myMesh = nullptr;


	// 노드에 메시가 있는지 확인
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* mesh = node->GetMesh();

		bool IsSkined = false;
		if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0) {
			IsSkined = true;
			myMesh = new SkinnedMesh;
		}
		else myMesh = new NormalMesh;


		// 이름
		myMesh->name = node->GetName();

		FbxVector4 min, max, center;
		node->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);

		// bounding box
		myMesh->min = XMFLOAT3(static_cast<float>(min[0]), static_cast<float>(min[1]), static_cast<float>(min[2]));
		myMesh->max = XMFLOAT3(static_cast<float>(max[0]), static_cast<float>(max[1]), static_cast<float>(max[2]));
		myMesh->center = XMFLOAT3(static_cast<float>(center[0]), static_cast<float>(center[1]), static_cast<float>(center[2]));

		// local matrix
		FbxMatrix mat = node->EvaluateGlobalTransform();
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				myMesh->localTransform.m[i][j] = static_cast<float>(mat.Get(i, j));



		// 정점 추출
		if (IsSkined) ExtractVertices(mesh, ((SkinnedMesh*)myMesh)->vertices, node->GetNodeAttribute());
		else  ExtractVertices(mesh, ((NormalMesh*)myMesh)->vertices, node->GetNodeAttribute());
#ifndef ALL_IN_ONE
		// 인덱스 추출
		ExtractIndices(mesh, myMesh.indices);
#endif

	}
	else if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		return nullptr;
	else {
		// empty node
		myMesh = new NormalMesh;
	}

	// 자식 노드로 재귀 호출
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		MeshBase* newMesh = TraverseNode(node->GetChild(i));

		if (newMesh != nullptr)
			myMesh->subMeshes.push_back(newMesh);
	}

	return myMesh;
}

// FBX 메시에서 정점을 추출하는 함수
void ExtractVertices(FbxMesh* mesh, std::vector<Vertex>& vertices, FbxNodeAttribute* attribute)
{
	// 정점의 수 가져오기
	int vertexCount = mesh->GetControlPointsCount();

#ifndef ALL_IN_ONE
	// 정점을 Vertex 구조체로 변환하여 벡터에 추가
	for (int i = 0; i < vertexCount; ++i)
	{
		FbxVector4 position = controlPoints[i];

		// DirectX 12에서 사용할 형식으로 변환하여 벡터에 추가
		Vertex vertex;
		vertex.position = XMFLOAT3(static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]));
		// 법선 정보가 있는 경우에만 추출
		if (mesh->GetElementNormalCount() > 0)
		{
			FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
			FbxVector4 normal;

			normal = normalElement->GetDirectArray().GetAt(i);
			vertex.normal = XMFLOAT3(static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]));

		}

		// 탄젠트 정보가 있는 경우에만 추출
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
				std::cout << "Hit Binormal!!! 주의!!!!!!!" << std::endl;
				neverHit = false;
			}

			// 주의!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			XMFLOAT3 tangent;
			XMStoreFloat3(&tangent, vectorTangent);
			vertex.tangent = tangent;
		}

		// 텍스처 좌표 정보가 있는 경우에만 추출
		if (mesh->GetElementUVCount() > 0)
		{
			FbxGeometryElementUV* uvElement = mesh->GetElementUV();
			FbxVector2 uv;
			uv = uvElement->GetDirectArray().GetAt(i);
			vertex.uv = XMFLOAT2(static_cast<float>(uv[0]), static_cast<float>(uv[1]));
		}

		vertices.push_back(vertex);
	}


#else
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {

		FbxMesh* mesh = static_cast<FbxMesh*>(attribute);

		int polygonCount = mesh->GetPolygonCount();
		int vertexCount = mesh->GetControlPointsCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();
		FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
		FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();
		FbxGeometryElementUV* uvElement = mesh->GetElementUV();


		std::cout << "vtx count : " << vertexCount << std::endl;
		std::cout << "nor count : " << mesh->GetElementNormalCount() << std::endl;
		std::cout << "tan count : " << mesh->GetElementTangentCount() << std::endl;
		std::cout << "uvs count : " << mesh->GetElementUVCount() << std::endl;
		std::cout << "uv mapping mode : " << mesh->GetElementUV()->GetMappingMode() << std::endl;
		std::cout << std::endl;

		for (int i = 0; i < polygonCount; ++i) {
			int polygonSize = mesh->GetPolygonSize(i);

			for (int j = 0; j < polygonSize; ++j) {
				int vertexIndex = mesh->GetPolygonVertex(i, j);

				if (vertexIndex < vertexCount) {
					Vertex vertex;
					vertex.position = { static_cast<float>(controlPoints[vertexIndex][0]), static_cast<float>(controlPoints[vertexIndex][1]), static_cast<float>(controlPoints[vertexIndex][2]) };

					if (normalElement) {
						FbxVector4 normal;// = normalElement->GetDirectArray().GetAt(normalIndex);
						mesh->GetPolygonVertexNormal(i, j, normal);
						vertex.normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };
					}

					if (tangentElement) {
						FbxLayerElement::EMappingMode tangentMappingMode = tangentElement->GetMappingMode();
						FbxVector4 tangent;

						if (tangentMappingMode == FbxLayerElement::eByControlPoint) {
							//tangent = tangentElement->GetDirectArray().GetAt(i * polygonSize + j);
							//printf("asdf");
						}
						else if (tangentMappingMode == FbxLayerElement::eByPolygonVertex) {
							int tangentIndex = tangentElement->GetIndexArray().GetAt(vertexIndex);// mesh->GetTextureUVIndex(i, j);
							//int tangentIndex = tangentElement->GetIndexArray().GetAt(vertexIndex);
							tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
						}
						vertex.tangent = { static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), static_cast<float>(tangent[2]) };

					}

					if (uvElement) {
						int uvIndex = mesh->GetTextureUVIndex(i, j);
						FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
						vertex.uv = { static_cast<float>(uv[0]), static_cast<float>(uv[1]) };
					}

					vertices.push_back(vertex);
				}
			}
		}
	}

#endif
}

void ExtractVertices(FbxMesh* mesh, std::vector<SkinnedVertex>& vertices, FbxNodeAttribute* attribute) {
	// 정점의 수 가져오기
	int vertexCount = mesh->GetControlPointsCount();
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {

		FbxMesh* mesh = static_cast<FbxMesh*>(attribute);

		int polygonCount = mesh->GetPolygonCount();
		int vertexCount = mesh->GetControlPointsCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();
		FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
		FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();
		FbxGeometryElementUV* uvElement = mesh->GetElementUV();

		for (int i = 0; i < polygonCount; ++i) {
			int polygonSize = mesh->GetPolygonSize(i);

			for (int j = 0; j < polygonSize; ++j) {
				int vertexIndex = mesh->GetPolygonVertex(i, j);

				if (vertexIndex < vertexCount) {
					SkinnedVertex vertex;
					vertex.position = { static_cast<float>(controlPoints[vertexIndex][0]), static_cast<float>(controlPoints[vertexIndex][1]), static_cast<float>(controlPoints[vertexIndex][2]) };

					if (normalElement) {
						FbxVector4 normal;
						mesh->GetPolygonVertexNormal(i, j, normal);
						vertex.normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };
					}

					if (tangentElement) {
						FbxLayerElement::EMappingMode tangentMappingMode = tangentElement->GetMappingMode();
						FbxVector4 tangent;

						if (tangentMappingMode == FbxLayerElement::eByControlPoint) {
						}
						else if (tangentMappingMode == FbxLayerElement::eByPolygonVertex) {
							int tangentIndex = tangentElement->GetIndexArray().GetAt(vertexIndex);// mesh->GetTextureUVIndex(i, j);
							tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
						}
						vertex.tangent = { static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), static_cast<float>(tangent[2]) };

					}

					if (uvElement) {
						int uvIndex = mesh->GetTextureUVIndex(i, j);
						FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
						vertex.uv = { static_cast<float>(uv[0]), static_cast<float>(uv[1]) };
					}

					// 스키닝 정보를 추가
					ExtractSkinningData(mesh, vertexIndex, vertex);

					vertices.push_back(vertex);
				}
			}
		}
	}
}

// 스키닝 정보를 추출하여 SkinnedVertex에 추가하는 함수
void ExtractSkinningData(FbxMesh* mesh, int vertexIndex, SkinnedVertex& vertex) 
{
	int clusterCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

	for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(clusterIndex, FbxDeformer::eSkin));

		int clusterCount = skin->GetClusterCount();
		for (int j = 0; j < clusterCount; ++j) {
			FbxCluster* cluster = skin->GetCluster(j);
			FbxNode* boneNode = cluster->GetLink();

			if (!boneNode)
				continue;

			//std::find(g_Vector.begin(), g_Vector.end(), boneNode->GetName());
			
			int realIdx = -1;
			for (int i = 0; i < g_Vector.size(); ++i) {
				if (g_Vector[i] == boneNode->GetName()) {
					realIdx = i;
					break;
				}
			}

			if (realIdx < 0) {
				std::cout << "BOne ERROR!!!!" << std::endl;
				exit(1);
			}

			FbxAMatrix transformMatrix;
			cluster->GetTransformMatrix(transformMatrix);

			int* indices = cluster->GetControlPointIndices();
			double* weights = cluster->GetControlPointWeights();
			int indexCount = cluster->GetControlPointIndicesCount();

			int weightCnt = 0;
			for (int m = 0; m < indexCount; ++m) {
				if (indices[m] == vertexIndex) 
				{
					// first
					if (vertex.blendingFactor.x == 0 && vertex.blendingIndex.x == 0) {
						vertex.blendingIndex.x = realIdx;
						vertex.blendingFactor.x = weights[m];
					}
					else if (vertex.blendingFactor.y == 0 && vertex.blendingIndex.y == 0) {
						vertex.blendingIndex.y = realIdx;
						vertex.blendingFactor.y = weights[m];
					}
					else if (vertex.blendingFactor.z == 0 && vertex.blendingIndex.z == 0) {
						vertex.blendingIndex.z = realIdx;
						vertex.blendingFactor.z = weights[m];
					}
					else if (vertex.blendingFactor.w == 0 && vertex.blendingIndex.w == 0) {
						vertex.blendingIndex.w = realIdx;
						vertex.blendingFactor.w = weights[m];
					}
					else {
						vertex.blendingFactor.w += weights[m];
						std::cout << "Exeeded max Blending Count!!, factors: " << vertex.blendingFactor.x + vertex.blendingFactor.y + vertex.blendingFactor.z + vertex.blendingFactor.w << std::endl;
					}

					//std::cout << "Index: " << vertexIndex << ", Bone ID: " << j << ",  Influenced by Bone: " << boneNode->GetName() << ", Weight: " << weights[m] << "\n";
				}
			}


			//for (int k = 0; k < indexCount; ++k) 
			//{
			//	if (indices[k] == vertexIndex) {
			//		if (cluster->GetLinkMode() == FbxCluster::eAdditive)
			//			continue;

			//		// 클러스터에 속한 뼈의 인덱스와 가중치를 저장합니다.
			//		if (vertex.blendingIndex.x == -1) {
			//			vertex.blendingIndex.x = clusterIndex;
			//			vertex.blendingFactor.x = static_cast<float>(weights[k]);
			//		}
			//		else if (vertex.blendingIndex.y == -1) {
			//			vertex.blendingIndex.y = clusterIndex;
			//			vertex.blendingFactor.y = static_cast<float>(weights[k]);
			//		}
			//		else if (vertex.blendingIndex.z == -1) {
			//			vertex.blendingIndex.z = clusterIndex;
			//			vertex.blendingFactor.z = static_cast<float>(weights[k]);
			//		}
			//		else if (vertex.blendingIndex.w == -1) {
			//			vertex.blendingIndex.w = clusterIndex;
			//			vertex.blendingFactor.w = static_cast<float>(weights[k]);
			//		}
			//	}
			//}
		}
	}
}


// FBX 메시에서 인덱스를 추출하는 함수
void ExtractIndices(FbxMesh* mesh, std::vector<uint16_t>& indices)
{
	// 메시의 폴리곤 수 가져오기
	int polygonCount = mesh->GetPolygonCount();

	// 각 폴리곤마다 인덱스 추출
	for (int i = 0; i < polygonCount; ++i)
	{
		// 폴리곤의 버텍스 인덱스 수 가져오기
		int polygonSize = mesh->GetPolygonSize(i);

		// 폴리곤의 각 버텍스 인덱스를 추출하여 벡터에 추가
		for (int j = 0; j < polygonSize; ++j)
		{
			int vertexIndex = mesh->GetPolygonVertex(i, j);

			// DirectX 12에서 uint16_t 형식을 사용하므로 적절한 변환을 수행
			indices.push_back(static_cast<uint16_t>(vertexIndex));
		}
	}
}

// 그냥 출력하는 함수
void PrintMeshHierachy(const MeshBase* mesh)
{
	std::cout << std::format("name: {}\n", mesh->name);
	std::cout << std::format("vertex: {}\n", mesh->GetVtxSize());
#ifndef ALL_IN_ONE
	std::cout << "index: " << mesh.indices.size() << std::endl << std::endl;
#endif
	//std::cout << "min: " << mesh.min.x << ", " << mesh.min.y << ", " << mesh.min.z << std::endl;
	//std::cout << "max: " << mesh.max.x << ", " << mesh.max.y << ", " << mesh.max.z << std::endl;
	//std::cout << "center: " << mesh.center.x << ", " << mesh.center.y << ", " << mesh.center.z << std::endl;

	//std::cout << std::endl;
	//std::cout << std::format("{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n",
	//	mesh.localTransform._11, mesh.localTransform._12, mesh.localTransform._13, mesh.localTransform._14,
	//	mesh.localTransform._21, mesh.localTransform._22, mesh.localTransform._23, mesh.localTransform._24,
	//	mesh.localTransform._31, mesh.localTransform._32, mesh.localTransform._33, mesh.localTransform._34,
	//	mesh.localTransform._41, mesh.localTransform._42, mesh.localTransform._43, mesh.localTransform._44		
	//	);

	for (const auto& m : mesh->subMeshes) {
		PrintMeshHierachy(m);
	}
}

// 파일로 출력하는 함수
void ExtractToFIle(const MeshBase* mesh, std::fstream& out)
{
	std::cout << std::format("name: {}\n", mesh->name);
	std::cout << std::format("vertex: {}\n\n", mesh->GetVtxSize());

	// 1 2 name
	WriteToFile(mesh->name, out);

	// 3 바운딩박스				// float3 x 3
	WriteToFile(mesh->min, out);
	WriteToFile(mesh->max, out);
	WriteToFile(mesh->center, out);

	// 4 matrix
	WriteToFile(mesh->localTransform, out);

	// 버텍스 정보				// int, position, normal, tangent, uv, int, index
#ifdef ALL_IN_ONE
	// 5, 6
	mesh->WriteVtxInfoAndData(out);

#else
	WriteToFile(0, out);

	WriteToFile(mesh.vertices.size(), out);
	for (const auto& vtx : mesh.vertices) {
		WriteToFile(vtx.position, out);
	}

	WriteToFile(mesh.vertices.size(), out);
	for (const auto& vtx : mesh.vertices) {
		WriteToFile(vtx.normal, out);
	}

	WriteToFile(mesh.vertices.size(), out);
	for (const auto& vtx : mesh.vertices) {
		WriteToFile(vtx.tangent, out);
	}

	WriteToFile(mesh.vertices.size(), out);
	for (const auto& vtx : mesh.vertices) {
		WriteToFile(vtx.uv, out);
	}

	// 7인덱스 정보
	WriteToFile(mesh.indices.size(), out);
	for (int i : mesh.indices) {
		WriteToFile(i, out);
	}
#endif

	// 서브메쉬 개수				// int
	WriteToFile(mesh->subMeshes.size(), out);
	for (const auto& m : mesh->subMeshes) {
		ExtractToFIle(m, out);
	}
}

std::string ChangeExtensionToBin(const std::string& originalFileName)
{
	size_t dotPosition = originalFileName.find_last_of('.');
	if (dotPosition != std::string::npos) {
		// 확장자가 있는 경우
		std::string fileNameWithoutExtension = originalFileName.substr(0, dotPosition);
#ifdef BINARY
		return fileNameWithoutExtension + ".bin";
#else
		return fileNameWithoutExtension + ".txt";
#endif
	}
	else {
		// 확장자가 없는 경우
#ifdef BINARY
		return originalFileName + ".bin";
#else
		return originalFileName + ".txt";
#endif
	}
}

void WriteToFile(unsigned int i, std::fstream& out)
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

void WriteToFile(const XMFLOAT4X4& matrix, std::fstream& out)
{
#ifdef BINARY
	out.write((const char*)(&matrix), sizeof(XMFLOAT4X4));
#else
	out << float2.x << ", " << float2.y << "\n";
#endif
}

void WriteToFile(const Vertex& vtx, std::fstream& out)
{
	out.write((const char*)&vtx, sizeof(Vertex));
}
