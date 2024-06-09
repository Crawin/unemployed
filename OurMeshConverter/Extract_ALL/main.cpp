#define BINARY

#include <iostream>
#include <string>
#include <format>
#include <direct.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <fbxsdk.h>
#include <DirectXMath.h>
#include "json/json.h"


// 우리의 메쉬 순서
// 이름 길이					// int
// 이름						// char*
// 바운딩박스				// float3 x 3
// 부모 상대 변환 행렬		// float4x4		// 해당 행렬은 사용하지 않을까 싶은데 어쩔까
// 버텍스 타입				// int
// 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 인덱스 정보				// int int*int
// 서브메쉬 개수				// int
// 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)
//

#define ALL_IN_ONE
#define MAX_CONTROL_POINT 4
using namespace DirectX;

enum MESH_TYPE {
	NONE = 0,
	NORMAL = 1,
	SKINNED = 2
};

enum JSON_FILE_OUT_TYPE {
	JSON_ALL = 0,
	JSON_COLLIDER_ONLY = 1,
	JSON_RENDERER_ONLY = 2,
};

struct Vertex
{
	XMFLOAT3 position = { 0,0,0 };
	XMFLOAT3 normal = { 0,0,0 };
	XMFLOAT3 tangent = { 0,0,0 };
	XMFLOAT2 uv = { 0,0 };
};

struct SkinnedVertex : public Vertex
{
	XMUINT4 blendingIndex = { 0,0,0,0 };
	XMFLOAT4 blendingFactor = { 0,0,0,0 };
};

struct Object {
	std::string m_Name = "";

	XMFLOAT3 m_BoundingBoxMin = { 0,0,0 };
	XMFLOAT3 m_BoundingBoxMax = { 0,0,0 };
	XMFLOAT3 m_BoundingBoxCenter = { 0,0,0 };

	XMFLOAT4X4 m_LocalTransform;

	XMFLOAT3 m_Transform = { 0,0,0 };
	XMFLOAT3 m_Rotate = { 0,0,0 };
	XMFLOAT3 m_Scale = { 1,1,1 };

	std::vector<Object*> m_Children;

	MESH_TYPE m_MeshType = NONE;
	std::vector<Vertex*> m_Vertices;
};

std::vector<std::string> g_Vector;
std::vector<XMFLOAT4X4> g_BoneMatrixVector;
XMFLOAT4X4 g_GlobalSettingTransform = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, };
XMMATRIX g_GlobalMat;
float g_GlobalImportScaleFactor = 1.0f;
bool g_ToOtherHandedSystem = false;
std::string g_FileName = "";
JSON_FILE_OUT_TYPE g_JsonFileOutOpt = JSON_ALL;

// for bone file
void SetBoneIndexSet(FbxNode* node);
void FindBone(FbxNode* Node);

// for file name setting
std::string RemoveExtension(const std::string& filename);

Object* TraverseNode(FbxNode* node);
void ExtractSkinningData(FbxMesh* mesh, int vertexIndex, SkinnedVertex* vertex);
void ExtractVertices(FbxMesh* mesh, Object* obj);

// for mesh
void BuildMeshFile(Object* obj, const std::string& originFileName);
void ExtractMesh(Object* obj, std::fstream& out, const XMFLOAT4X4* parent = nullptr);

// Function to compute the local bounding box
void ComputeLocalBoundingBox(FbxNode* pNode, XMFLOAT3& min, XMFLOAT3& max, XMFLOAT3& center);

// for json
void BuildJsonFile(Object* obj, const std::string& originFileName);
Json::Value ExtractObjectJson(Object* obj);

// to write
void WriteToFile(unsigned int i, std::fstream& out);
void WriteToFile(const std::string& str, std::fstream& out);
void WriteToFile(const XMFLOAT2& float2, std::fstream& out);
void WriteToFile(const XMFLOAT3& float3, std::fstream& out);
void WriteToFile(const XMFLOAT4X4& matrix, std::fstream& out);

int main(int argc, char* argv[])
{
	_mkdir("Result");

	// FBX SDK 초기화
	FbxManager* fbxManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ios);

	// FBX 파일 로드
	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

	// 1 : file name
	// 2 : import scale factor 
	const char* fileName = argv[1];
	//if (argc > 2) g_GlobalImportScaleFactor = atof(argv[2]);
	if (argc >= 2) {
		for (int i = 1; i < argc; ++i) {
			if (strcmp(argv[i], "-b") == 0) g_JsonFileOutOpt = JSON_COLLIDER_ONLY;
			else if (strcmp(argv[i], "-r") == 0) g_JsonFileOutOpt = JSON_RENDERER_ONLY;
			else if (strcmp(argv[i], "-s") == 0) {
				// scale
				if (i + 1 < argc) g_GlobalImportScaleFactor = atof(argv[++i]);
				else {
					printf("ERROR!!!!, not enough args");
					exit(-1);
				}
			}
		}
	}
	//const char* fileName = "map-Emaintest.fbx";

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

	// convert to left handed
	//scene->GetGlobalSettings().SetAxisSystem(axisSystem);
	
	FbxAMatrix matTemp;
	FbxAxisSystem axisSystem(FbxAxisSystem::eDirectX);
	axisSystem.GetMatrix(matTemp);
	printf("ToMat\n");
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			g_GlobalSettingTransform.m[i][j] = static_cast<float>(matTemp.Get(i, j));
			printf("%.1f, ", matTemp.Get(i, j));
		}
		printf("\n");
	}
	printf("\n");

	FbxAMatrix globalMat;
	scene->GetGlobalSettings().GetAxisSystem().GetMatrix(globalMat);
	printf("GlobalMat\n");
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%.1f, ", globalMat.Get(i, j));
		}
		printf("\n");
	}
	printf("\n");

	g_GlobalMat = XMLoadFloat4x4(&g_GlobalSettingTransform);

	g_ToOtherHandedSystem = scene->GetGlobalSettings().GetAxisSystem().GetCoorSystem() == FbxAxisSystem::eRightHanded;

	// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
	FbxGeometryConverter geometryConverter(fbxManager);
	geometryConverter.Triangulate(scene, true);

	importer->Destroy();

	// 노드 탐색 및 정점 및 인덱스 추출
	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode == nullptr) return -1;
	// find skeleton first
	FindBone(rootNode);

	// if bone, export to file
	if (g_BoneMatrixVector.size() > 0) {
		// export bone file
		_mkdir("Result\\Bone");
		std::string boneFileName = "Result\\Bone\\bone_";
		boneFileName += RemoveExtension(fileName);
		boneFileName += ".bin";

		std::fstream output(boneFileName, std::ios::binary | std::ios::out);

		WriteToFile((int)(g_BoneMatrixVector.size()), output);
		output.write((const char*)(&g_BoneMatrixVector[0]), sizeof(XMFLOAT4X4) * g_BoneMatrixVector.size());
	}

	Object* obj = new Object;
	std::string name = RemoveExtension(fileName);
	g_FileName = name;
	obj->m_Name = name + "_Root";

	for (int i = 0; i < rootNode->GetChildCount(); ++i)
	{
		FbxNode* child = rootNode->GetChild(i);
		FbxNodeAttribute* attrib = child->GetNodeAttribute();
		if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			continue;

		Object* t = TraverseNode(child);
		if (t != nullptr) obj->m_Children.push_back(t);
	}

	BuildMeshFile(obj, name);
	BuildJsonFile(obj, name);

	// 정리 작업
	fbxManager->Destroy();

	std::cout << "Complete!!" << std::endl;
	return 0;
}

// bone
void SetBoneIndexSet(FbxNode* node)
{
	// no bone
	if (g_Vector.end() == std::find(g_Vector.begin(), g_Vector.end(), node->GetName())) {
		std::cout << "IDX : " << g_Vector.size() << ",\tSkeleton node name : " << node->GetName() << std::endl;
		XMFLOAT4X4 trans;
		FbxMatrix fbxTransform = node->EvaluateGlobalTransform();
		FbxAMatrix fbxTransformTemp = node->EvaluateGlobalTransform();
		FbxVector4 t = fbxTransformTemp.GetT();

		std::cout << "Translation: (" << t[0] << ", " << t[1] << ", " << t[2] << ")" << std::endl;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				trans.m[i][j] = fbxTransformTemp[i][j];
			}
		}

		// should transpose
		// to left handed
		XMFLOAT4X4 left =
		{
			-1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};
		XMMATRIX convertLeft = XMLoadFloat4x4(&left);
		XMMATRIX boneMat = XMLoadFloat4x4(&trans);

		// to root
		XMMATRIX inv = XMMatrixInverse(nullptr, boneMat * g_GlobalMat);
		inv = XMMatrixTranspose(inv);
		XMStoreFloat4x4(&trans, inv);

		// insert data to vector
		g_Vector.push_back(node->GetName());
		g_BoneMatrixVector.push_back(trans);
	}

	// DFS로 탐색한다.
	for (int i = 0; i < node->GetChildCount(); ++i) {
		SetBoneIndexSet(node->GetChild(i));
	}
}

// bone
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

// ext
std::string RemoveExtension(const std::string& filename)
{
	size_t lastDotPos = filename.find_last_of('.');
	if (lastDotPos != std::string::npos) { // '.'이 발견되면
		return filename.substr(0, lastDotPos); // '.' 이전의 부분을 반환
	}
	return filename; // '.'이 발견되지 않으면 원래 문자열을 그대로 반환
}

Object* TraverseNode(FbxNode* node)
{
	Object* obj = new Object;
	
	obj->m_Name = node->GetName();
	obj->m_MeshType = NONE;

	// convert this to local....
	FbxVector4 t = node->EvaluateLocalTranslation();
	FbxVector4 r = node->EvaluateLocalRotation();
	FbxVector4 s = node->EvaluateLocalScaling();

	obj->m_Transform = { static_cast<float>(t[0]) / g_GlobalImportScaleFactor, static_cast<float>(t[1]) / g_GlobalImportScaleFactor, static_cast<float>(t[2]) / g_GlobalImportScaleFactor };
	obj->m_Rotate = { static_cast<float>(r[0]), static_cast<float>(r[1]), static_cast<float>(r[2]) };
	obj->m_Scale = { static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2]) };

	ComputeLocalBoundingBox(node, obj->m_BoundingBoxMin, obj->m_BoundingBoxMax, obj->m_BoundingBoxCenter);

	XMMATRIX tempMat = {
		-1,0,0,0,
		0,-1,0,0,
		0,0,-1,0,
		0,0,0,1,
	};

	XMStoreFloat3(&obj->m_Transform, XMVector3Transform(XMLoadFloat3(&obj->m_Transform), g_GlobalMat));
	XMStoreFloat3(&obj->m_Rotate, XMVector3Transform(XMLoadFloat3(&obj->m_Rotate), g_GlobalMat * tempMat));

	XMStoreFloat3(&obj->m_BoundingBoxMin, XMVector3Transform(XMLoadFloat3(&obj->m_BoundingBoxMin), g_GlobalMat));
	XMStoreFloat3(&obj->m_BoundingBoxMax, XMVector3Transform(XMLoadFloat3(&obj->m_BoundingBoxMax), g_GlobalMat));
	XMStoreFloat3(&obj->m_BoundingBoxCenter, XMVector3Transform(XMLoadFloat3(&obj->m_BoundingBoxCenter), g_GlobalMat));

	XMMATRIX matT = XMMatrixTranslationFromVector(XMLoadFloat3(&obj->m_Transform));
	XMMATRIX matR = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&obj->m_Rotate));
	XMMATRIX matS = XMMatrixScalingFromVector(XMLoadFloat3(&obj->m_Scale));

	XMStoreFloat4x4(&obj->m_LocalTransform, matS * matR * matT);

	// 노드에 메시가 있는지 확인
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute != nullptr && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
	
		FbxMesh* mesh = static_cast<FbxMesh*>(attribute);
		//FbxMesh* mesh = node->GetMesh();

		ExtractVertices(mesh, obj);
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		Object* newObj = TraverseNode(node->GetChild(i));

		if (newObj != nullptr)
			obj->m_Children.push_back(newObj);
	}

	return obj;
}

void ExtractVertices(FbxMesh* mesh, Object* obj)
{
	if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0)
		obj->m_MeshType = SKINNED;
	else 
		obj->m_MeshType = NORMAL;

	int polygonCount = mesh->GetPolygonCount();
	int vertexCount = mesh->GetControlPointsCount();
	FbxVector4* controlPoints = mesh->GetControlPoints();
	FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
	FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();
	FbxGeometryElementUV* uvElement = mesh->GetElementUV();

	bool isSkinned = obj->m_MeshType == SKINNED;

	for (int i = 0; i < polygonCount; ++i) {
		int polygonSize = mesh->GetPolygonSize(i);

		for (int j = 0; j < polygonSize; ++j) {
			int vertexIndex = mesh->GetPolygonVertex(i, j);

			if (vertexIndex < vertexCount) {
				Vertex* vertex = isSkinned ? new SkinnedVertex : new Vertex;
				vertex->position = { static_cast<float>(controlPoints[vertexIndex][0]), static_cast<float>(controlPoints[vertexIndex][1]), static_cast<float>(controlPoints[vertexIndex][2]) };

				if (normalElement) {
					FbxVector4 normal;
					mesh->GetPolygonVertexNormal(i, j, normal);
					vertex->normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };
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
					vertex->tangent = { static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), static_cast<float>(tangent[2]) };

				}

				if (uvElement) {
					int uvIndex = mesh->GetTextureUVIndex(i, j);
					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
					vertex->uv = { static_cast<float>(uv[0]), static_cast<float>(uv[1]) };
				}

				if (isSkinned) ExtractSkinningData(mesh, vertexIndex, (SkinnedVertex*)vertex);
				
				obj->m_Vertices.push_back(vertex);
			}
		}
	}

	if (obj->m_Vertices.size() == 0) return;

	// set scale
	for (auto& v : obj->m_Vertices) {
		v->position.x /= g_GlobalImportScaleFactor;
		v->position.y /= g_GlobalImportScaleFactor;
		v->position.z /= g_GlobalImportScaleFactor;
	}

	// if convert to left handed
	if (g_ToOtherHandedSystem) {
		for (int i = 0; i < obj->m_Vertices.size(); i += 3) {
			std::swap(obj->m_Vertices[i + 1], obj->m_Vertices[i + 2]);
		}
	}

	// convert to DirectX axis system (lefthanded, x: right, y: up, z: foward)
	for (int i = 0; i < obj->m_Vertices.size(); ++i) {
		XMStoreFloat3(&obj->m_Vertices[i]->position, XMVector3Transform(XMLoadFloat3(&obj->m_Vertices[i]->position), g_GlobalMat));
		XMStoreFloat3(&obj->m_Vertices[i]->normal, XMVector3Transform(XMLoadFloat3(&obj->m_Vertices[i]->normal), g_GlobalMat));
		XMStoreFloat3(&obj->m_Vertices[i]->tangent, XMVector3Transform(XMLoadFloat3(&obj->m_Vertices[i]->tangent), g_GlobalMat));
	}

	obj->m_BoundingBoxCenter.x /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxCenter.y /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxCenter.z /= g_GlobalImportScaleFactor;

	obj->m_BoundingBoxMax.x /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxMax.y /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxMax.z /= g_GlobalImportScaleFactor;
	
	obj->m_BoundingBoxMin.x /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxMin.y /= g_GlobalImportScaleFactor;
	obj->m_BoundingBoxMin.z /= g_GlobalImportScaleFactor;

	// set bounding box
	//XMFLOAT3 min = { 
	//	std::numeric_limits<float>::infinity(), 
	//	std::numeric_limits<float>::infinity(), 
	//	std::numeric_limits<float>::infinity() 
	//};
	//XMFLOAT3 max = {
	//	-1.0f * std::numeric_limits<float>::infinity(),
	//	-1.0f * std::numeric_limits<float>::infinity(),
	//	-1.0f * std::numeric_limits<float>::infinity()
	//};
	//XMFLOAT3 center = { 0,0,0 };

	//for (auto& v : obj->m_Vertices) {
	//	min.x = std::min(min.x, v->position.x);
	//	min.y = std::min(min.y, v->position.y);
	//	min.z = std::min(min.z, v->position.z);

	//	max.x = std::max(min.x, v->position.x);
	//	max.y = std::max(min.y, v->position.y);
	//	max.z = std::max(min.z, v->position.z);
	//}

	//XMStoreFloat3(&center, (XMLoadFloat3(&min) + XMLoadFloat3(&max)) / 2.0f);

	//obj->m_BoundingBoxCenter = center;
	//obj->m_BoundingBoxMax = max;
	//obj->m_BoundingBoxMin = min;
}

// 스키닝 정보를 추출하여 SkinnedVertex에 추가하는 함수
void ExtractSkinningData(FbxMesh* mesh, int vertexIndex, SkinnedVertex* vertex)
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
					if (vertex->blendingFactor.x == 0 && vertex->blendingIndex.x == 0) {
						vertex->blendingIndex.x = realIdx;
						vertex->blendingFactor.x = weights[m];
					}
					else if (vertex->blendingFactor.y == 0 && vertex->blendingIndex.y == 0) {
						vertex->blendingIndex.y = realIdx;
						vertex->blendingFactor.y = weights[m];
					}
					else if (vertex->blendingFactor.z == 0 && vertex->blendingIndex.z == 0) {
						vertex->blendingIndex.z = realIdx;
						vertex->blendingFactor.z = weights[m];
					}
					else if (vertex->blendingFactor.w == 0 && vertex->blendingIndex.w == 0) {
						vertex->blendingIndex.w = realIdx;
						vertex->blendingFactor.w = weights[m];
					}
					else {
						vertex->blendingFactor.w += weights[m];
						std::cout << "Exeeded max Blending Count!!, factors: " << vertex->blendingFactor.x + vertex->blendingFactor.y + vertex->blendingFactor.z + vertex->blendingFactor.w << std::endl;
					}
				}
			}
		}
	}
}

// Function to compute the local bounding box
void ComputeLocalBoundingBox(FbxNode* pNode, XMFLOAT3& min, XMFLOAT3& max, XMFLOAT3& center)
{
	// Step 1: Get the global bounding box
	FbxVector4 globalMin, globalMax, globalCenter;
	pNode->EvaluateGlobalBoundingBoxMinMaxCenter(globalMin, globalMax, globalCenter);

	// Step 2: Get the global transformation matrix and compute its inverse
	FbxAMatrix globalTransform = pNode->EvaluateGlobalTransform();
	FbxAMatrix inverseGlobalTransform = globalTransform.Inverse();

	// Step 3: Transform global bounding box vertices to local space
	FbxVector4 localMin = inverseGlobalTransform.MultT(globalMin);
	FbxVector4 localMax = inverseGlobalTransform.MultT(globalMax);
	FbxVector4 localCenter = inverseGlobalTransform.MultT(globalCenter);

	// Step 4: Set the output parameters
	min = { static_cast<float>(localMin[0]), static_cast<float>(localMin[1]), static_cast<float>(localMin[2]) };
	max = { static_cast<float>(localMax[0]), static_cast<float>(localMax[1]), static_cast<float>(localMax[2])};
	center = { static_cast<float>(localCenter[0]), static_cast<float>(localCenter[1]), static_cast<float>(localCenter[2])};
}

void BuildMeshFile(Object* obj, const std::string& meshName)
{
	_mkdir("Result\\Mesh");
	std::string outputFileName = meshName + ".bin";
	std::string of = "Result\\Mesh\\" + outputFileName;
	std::fstream out(of, std::ios::binary | std::ios::out);

	ExtractMesh(obj, out, nullptr);
}

void ExtractMesh(Object* obj, std::fstream& out, const XMFLOAT4X4* parent)
{
	// 1 2 name
	WriteToFile(obj->m_Name, out);

	// 3 바운딩박스				// float3 x 3
	WriteToFile(obj->m_BoundingBoxMin, out);
	WriteToFile(obj->m_BoundingBoxMax, out);
	WriteToFile(obj->m_BoundingBoxCenter, out);

	XMFLOAT3 ext = {
		obj->m_BoundingBoxMax.x - obj->m_BoundingBoxMin.x,
		obj->m_BoundingBoxMax.y - obj->m_BoundingBoxMin.y,
		obj->m_BoundingBoxMax.z - obj->m_BoundingBoxMin.z
	};
	//printf("%s\n", obj->m_Name.c_str());
	//printf("%.1f, %.1f, %.1f\n",	obj->m_BoundingBoxCenter.x, obj->m_BoundingBoxCenter.y, obj->m_BoundingBoxCenter.z);
	//printf("%.1f, %.1f, %.1f\n\n", ext.x, ext.y, ext.z);

	// 4 matrix
	if (parent == nullptr) {
		WriteToFile(obj->m_LocalTransform, out);
	}
	else {
		XMMATRIX p = XMLoadFloat4x4(parent);
		XMMATRIX c = XMLoadFloat4x4(&obj->m_LocalTransform);
		XMMATRIX r = c * p;
		XMFLOAT4X4 t;
		XMStoreFloat4x4(&t, r);
		WriteToFile(t, out);
	}

	// 5
	int meshType = obj->m_MeshType;
	WriteToFile(meshType, out);

	// 6 - 1
	WriteToFile(obj->m_Vertices.size(), out);

	// 6 - 2
	if (obj->m_Vertices.size() > 0) {
		int size = 0;
		switch (obj->m_MeshType) {
		case NORMAL:
			size = sizeof(Vertex);
			break;
		case SKINNED:
			size = sizeof(SkinnedVertex);
			break;
		default:
			printf("ERROR");
			break;
		}

		for (int i = 0; i < obj->m_Vertices.size(); ++i) 
			out.write((const char*)(obj->m_Vertices[i]), size);
	}

	// 7
	WriteToFile(obj->m_Children.size(), out);
	for (const auto& child : obj->m_Children) {
		if (parent == nullptr) {
			ExtractMesh(child, out, &obj->m_LocalTransform);// , & mesh->localTransform);
		}
		else {
			XMMATRIX p = XMLoadFloat4x4(parent);
			XMMATRIX c = XMLoadFloat4x4(&obj->m_LocalTransform);
			XMMATRIX r = c * p;
			XMFLOAT4X4 t;
			XMStoreFloat4x4(&t, r);
			ExtractMesh(child, out, &t);
		}
	}

}

void BuildJsonFile(Object* obj, const std::string& meshName)
{
	_mkdir("Result\\Object");
	std::string outputFileName = meshName + ".json";
	std::string of = "Result\\Object\\" + outputFileName;
	std::fstream outputFile(of, std::ios::out);

	Json::Value res = ExtractObjectJson(obj);

	outputFile << res;

}

Json::Value ExtractObjectJson(Object* obj)
{
	Json::Value val;

	// name
	val["Name"] = obj->m_Name;
	
	// transform
	Json::Value transform;
	Json::Value pos;
	pos.append(obj->m_Transform.x);
	pos.append(obj->m_Transform.y);
	pos.append(obj->m_Transform.z);
	transform["Position"] = pos;

	Json::Value rot;
	rot.append(obj->m_Rotate.x);
	rot.append(obj->m_Rotate.y);
	rot.append(obj->m_Rotate.z);
	transform["Rotate"] = rot;

	Json::Value sca;
	sca.append(obj->m_Scale.x);
	sca.append(obj->m_Scale.y);
	sca.append(obj->m_Scale.z);
	transform["Scale"] = sca;

	val["Transform"] = transform;

	// renderer
	// todo material name까지 추출 가능 하게
	if (obj->m_Vertices.size() > 0) {
		Json::Value renderer;
		renderer["Mesh"] = g_FileName + "." + obj->m_Name;
		renderer["Material"] = "NoMaterial";

		if (g_JsonFileOutOpt == JSON_ALL || g_JsonFileOutOpt == JSON_RENDERER_ONLY)
			val["Renderer"] = renderer;
	
		// collider
		Json::Value collider;
		collider["Static"] = true;
		
		// manual
		if (g_JsonFileOutOpt == JSON_COLLIDER_ONLY) {
			collider["AutoMesh"] = false;

			XMFLOAT3 ext = {
				obj->m_BoundingBoxMax.x - obj->m_BoundingBoxMin.x,
				obj->m_BoundingBoxMax.y - obj->m_BoundingBoxMin.y,
				obj->m_BoundingBoxMax.z - obj->m_BoundingBoxMin.z
			};
			ext.x = abs(ext.x);
			ext.y = abs(ext.y);
			ext.z = abs(ext.z);

			// center
			Json::Value center;
			center.append(obj->m_BoundingBoxCenter.x);
			center.append(obj->m_BoundingBoxCenter.y);
			center.append(obj->m_BoundingBoxCenter.z);
			collider["Center"] = center;

			// todo
			// if name contains stair, ext.y = 0.1, rotate = "calculated angle"
			//std::string::size_type p = obj->m_Name.find("stair");
			//if (p != std::string::npos) printf("stair!!");

			// extent
			Json::Value extent;
			extent.append(ext.x);
			extent.append(ext.y);
			extent.append(ext.z);
			collider["Extent"] = extent;

			// rotate
			Json::Value rotate;
			rotate.append(0.0f);
			rotate.append(0.0f);
			rotate.append(0.0f);
			collider["Rotate"] = rotate;
		}
		else collider["AutoMesh"] = true;

		if (g_JsonFileOutOpt == JSON_ALL || g_JsonFileOutOpt == JSON_COLLIDER_ONLY)
			val["Collider"] = collider;
	}

	if (obj->m_Children.size() > 0) {
		Json::Value children;

		for (Object* child : obj->m_Children)
			children.append(ExtractObjectJson(child));

		val["Children"] = children;
	}

	return val;
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
