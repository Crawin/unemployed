#define FBXSDK_SHARED
#include <fbxsdk.h>
#include <DirectXMath.h>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <vector>

using namespace DirectX;

std::vector<std::string> g_Vector;

bool g_LeftHanded = false;
int g_Count = 0;

std::string removeExtension(const std::string& filename) {
	size_t lastDotPos = filename.find_last_of('.');
	if (lastDotPos != std::string::npos) { // '.'이 발견되면
		return filename.substr(0, lastDotPos); // '.' 이전의 부분을 반환
	}
	return filename; // '.'이 발견되지 않으면 원래 문자열을 그대로 반환
}

void SetBoneIndexSet(FbxNode* node) {
	// no bone
	if (g_Vector.end() == std::find(g_Vector.begin(), g_Vector.end(), node->GetName())) {
		std::cout << "IDX : " << g_Vector.size() << ",\tSkeleton node name : " << node->GetName() << std::endl;
		// insert data to vector
		g_Vector.push_back(node->GetName());
	}

	// DFS로 탐색한다.
	for (int i = 0; i < node->GetChildCount(); ++i) {
		SetBoneIndexSet(node->GetChild(i));
	}
}

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

// Function to recursively print node hierarchy and animation data
void PrintNodeHierarchy(FbxNode* pNode, std::fstream& out, FbxAnimStack* pAnimStack = nullptr) {
	if (!pNode) return;

	FbxString nodeName = pNode->GetName();
	
	std::string name = nodeName.Buffer();

	// if fit
	static int printCount = 0;
	if (g_Vector.size() > g_Count && name == g_Vector[g_Count])
	{
		++g_Count;
		std::cout << "Node Name: " << nodeName.Buffer() << std::endl;

		if (pAnimStack) {
			FbxTimeSpan timeSpan = pAnimStack->GetLocalTimeSpan();
			FbxTime start = timeSpan.GetStart();
			FbxTime end = timeSpan.GetStop();

			static bool isFirst = true;
			if (isFirst == true) {
				// 1. total frame
				unsigned int totalFrame = end.GetFrameCount(FbxTime::eFrames24);
				out.write((char*)(&totalFrame), sizeof(unsigned int));

				unsigned int boneSize = g_Vector.size();
				out.write((char*)(&boneSize), sizeof(unsigned int));

				isFirst = false;
			}

			std::cout << "StartFrame : " << start.GetFrameCount(FbxTime::eFrames24) << ", EndFrame : " << end.GetFrameCount(FbxTime::eFrames24) << std::endl;

			FbxTime time;
			time.SetFrame(0, FbxTime::eFrames24);
			FbxAMatrix temp = pNode->EvaluateGlobalTransform(time);
			FbxVector4 t = temp.GetT();
			//std::cout << "Translation: (" << t[0] << ", " << t[1] << ", " << t[2] << ")" << std::endl;

			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i) {
				FbxTime currentTime;
				currentTime.SetFrame(i, FbxTime::eFrames24);
				//std::cout << "Frame " << i << std::endl;

				// Get the local transformation matrix for the node at the current time
				FbxAMatrix globalMatrix = pNode->EvaluateGlobalTransform(currentTime);
				FbxVector4 translation = globalMatrix.GetT();
				FbxVector4 rotation = globalMatrix.GetR();
				FbxVector4 scaling = globalMatrix.GetS();
				XMFLOAT4X4 trans;
				for (int i = 0; i < 4; ++i) {
					for (int j = 0; j < 4; ++j) {
						// should transpose
						trans.m[i][j] = globalMatrix[i][j];
					}
				}

				// to left handed
				if (g_LeftHanded == false) {
					XMFLOAT4X4 left =
					{
						-1,0,0,0,
						0,1,0,0,
						0,0,1,0,
						0,0,0,1
					};
					XMMATRIX convertLeft = XMLoadFloat4x4(&left);
					XMMATRIX boneMat = XMLoadFloat4x4(&trans);

					XMStoreFloat4x4(&trans, XMMatrixMultiply(boneMat, convertLeft));

					//std::cout << "to left handed\n";

				}



				XMStoreFloat4x4(&trans, XMMatrixTranspose(XMLoadFloat4x4(&trans)));

				out.write((char*)(&trans), sizeof(XMFLOAT4X4));

				/*XMFLOAT3 t = { (float)translation[0], (float)translation[1], (float)translation[2] };
				XMFLOAT3 r = { XMConvertToRadians(rotation[0]),  XMConvertToRadians(rotation[1]), XMConvertToRadians(rotation[2]) };
				XMFLOAT3 s = { (float)scaling[0], (float)scaling[1], (float)scaling[2] };

				XMMATRIX matrix =
					XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&t)),
						XMMatrixMultiply(XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&r)), XMMatrixScalingFromVector(XMLoadFloat3(&s))));*/

				// transpose
				//std::cout << printCount++ << std::endl;
				//std::cout << "Translation: (" << translation[0] << ", " << translation[1] << ", " << translation[2] << ")\n";// << std::endl;
				//std::cout << "Rotation: (" << rotation[0] << ", " << rotation[1] << ", " << rotation[2] << ")" << std::endl;
				//std::cout << "Scaling: (" << scaling[0] << ", " << scaling[1] << ", " << scaling[2] << ")" << std::endl;
			}
		}
	}

	// Also DFS
	int childCount = pNode->GetChildCount();
	for (int i = 0; i < childCount; ++i) {
		PrintNodeHierarchy(pNode->GetChild(i), out, pAnimStack);
	}
}

int main(int argc, char* argv[])
{
	_mkdir("Result");

	for (int i = 1; i < argc; ++i) {

		FbxManager* manager = FbxManager::Create();
		FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
		manager->SetIOSettings(ios);

		// Importer to load the FBX file
		FbxImporter* importer = FbxImporter::Create(manager, "");
		const char* filename = argv[i]; // Replace with your FBX file path
		if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
			std::cerr << "Failed to initialize importer!" << std::endl;
			std::cerr << "Error returned: " << importer->GetStatus().GetErrorString() << std::endl;
			return -1;
		}

		// Scene to hold the imported data
		FbxScene* scene = FbxScene::Create(manager, "MyScene");
		importer->Import(scene);

		if (scene->GetGlobalSettings().GetAxisSystem().GetCoorSystem() == FbxAxisSystem::eLeftHanded) g_LeftHanded = true;

		// Destroy importer after importing is done
		importer->Destroy();

		// Get the root node of the scene
		FbxNode* rootNode = scene->GetRootNode();

		FindBone(rootNode);

		// Get the number of animation stacks in the scene
		int numAnimStacks = scene->GetSrcObjectCount<FbxAnimStack>();
		if (numAnimStacks == 0) {
			std::cerr << "No animation stack found!" << std::endl;
			return -1;
		}

		// Iterate through each animation stack and print animation data
		for (int i = 0; i < numAnimStacks; ++i) {
			std::string t = "Result/anim_";
			t += removeExtension(filename) + ".bin";

			std::fstream outFile(t, std::ios::out | std::ios::binary);

			FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
			std::cout << "Animation Stack Name: " << animStack->GetName() << std::endl;
			PrintNodeHierarchy(rootNode, outFile, animStack);
		}

		// Destroy the scene and manager
		scene->Destroy();
		manager->Destroy();
	}
	return 0;
}
