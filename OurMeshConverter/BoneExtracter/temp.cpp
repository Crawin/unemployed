#define FBXSDK_SHARED
#include <fbxsdk.h>
#include <DirectXMath.h>
#include <iostream>
#include <fstream>

using namespace DirectX;

std::string removeExtension(const std::string& filename) {
	size_t lastDotPos = filename.find_last_of('.');
	if (lastDotPos != std::string::npos) { // '.'이 발견되면
		return filename.substr(0, lastDotPos); // '.' 이전의 부분을 반환
	}
	return filename; // '.'이 발견되지 않으면 원래 문자열을 그대로 반환
}

// Function to recursively print node hierarchy and animation data
void PrintNodeHierarchy(FbxNode* pNode, std::fstream& out, FbxAnimStack* pAnimStack = nullptr) {
	if (!pNode) return;

	FbxString nodeName = pNode->GetName();
	std::cout << "Node Name: " << nodeName.Buffer() << std::endl;

	if (pAnimStack) {
		FbxTimeSpan timeSpan = pAnimStack->GetLocalTimeSpan();
		FbxTime start = timeSpan.GetStart();
		FbxTime end = timeSpan.GetStop();

		std::cout << "StartFrame : " << start.GetFrameCount(FbxTime::eFrames24) << ", EndFrame : " << end.GetFrameCount(FbxTime::eFrames24) << std::endl;

		for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i) {
		    FbxTime currentTime;
		    currentTime.SetFrame(i, FbxTime::eFrames24);
		    std::cout << "Frame " << i << std::endl;

		    // Get the local transformation matrix for the node at the current time
		    FbxAMatrix localMatrix = pNode->EvaluateGlobalTransform(currentTime);
		    FbxVector4 translation = localMatrix.GetT();
		    FbxVector4 rotation = localMatrix.GetR();
		    FbxVector4 scaling = localMatrix.GetS();

			XMFLOAT3 t = { (float)translation[0], (float)translation[1], (float)translation[2] };
			XMFLOAT3 r = { XMConvertToRadians(rotation[0]),  XMConvertToRadians(rotation[1]), XMConvertToRadians(rotation[2]) };
			XMFLOAT3 s = { (float)scaling[0], (float)scaling[1], (float)scaling[2] };

			XMMATRIX matrix =
				XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&t)),
					XMMatrixMultiply(XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&r)), XMMatrixScalingFromVector(XMLoadFloat3(&s))));

			XMFLOAT4X4 mat;
			XMStoreFloat4x4(&mat, matrix);



		    std::cout << "Translation: (" << translation[0] << ", " << translation[1] << ", " << translation[2] << ")" << std::endl;
		    std::cout << "Rotation: (" << rotation[0] << ", " << rotation[1] << ", " << rotation[2] << ")" << std::endl;
		    std::cout << "Scaling: (" << scaling[0] << ", " << scaling[1] << ", " << scaling[2] << ")" << std::endl;
		}
	}

	// Also DFS
	int childCount = pNode->GetChildCount();
	for (int i = 0; i < childCount; ++i) {
		PrintNodeHierarchy(pNode->GetChild(i), out, pAnimStack);
	}
}

int main() 
{
	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	// Importer to load the FBX file
	FbxImporter* importer = FbxImporter::Create(manager, "");
	const char* filename = "dia_dance_without_skin.fbx"; // Replace with your FBX file path
	if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
		std::cerr << "Failed to initialize importer!" << std::endl;
		std::cerr << "Error returned: " << importer->GetStatus().GetErrorString() << std::endl;
		return -1;
	}

	// Scene to hold the imported data
	FbxScene* scene = FbxScene::Create(manager, "MyScene");
	importer->Import(scene);

	// Destroy importer after importing is done
	importer->Destroy();

	// Get the root node of the scene
	FbxNode* rootNode = scene->GetRootNode();

	// Get the number of animation stacks in the scene
	int numAnimStacks = scene->GetSrcObjectCount<FbxAnimStack>();
	if (numAnimStacks == 0) {
		std::cerr << "No animation stack found!" << std::endl;
		return -1;
	}

	// Iterate through each animation stack and print animation data
	for (int i = 0; i < numAnimStacks; ++i) {
		std::string t = "anim_";
		t += removeExtension(filename) + ".bin";

		std::fstream outFile("", std::ios::out | std::ios::binary);

		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
		std::cout << "Animation Stack Name: " << animStack->GetName() << std::endl;
		PrintNodeHierarchy(rootNode, outFile, animStack);
	}

	// Destroy the scene and manager
	scene->Destroy();
	manager->Destroy();

	return 0;
}
