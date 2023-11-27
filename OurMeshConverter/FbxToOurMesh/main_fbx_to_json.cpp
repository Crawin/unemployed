#include <fbxsdk.h>
#include <iostream>

void PrintNodeInfo(FbxNode* pNode);

void ExtractMeshInfo(FbxNode* pNode)
{
    if (!pNode)
        return;

    FbxNodeAttribute* nodeAttribute = pNode->GetNodeAttribute();

    if (nodeAttribute)
    {
        if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* mesh = pNode->GetMesh();
            if (mesh)
            {
                // 메시 정보 추출 예제
                int vertexCount = mesh->GetControlPointsCount();
                FbxVector4* controlPoints = mesh->GetControlPoints();

                std::cout << "Mesh Information:" << std::endl;
                std::cout << "Vertex Count: " << vertexCount << std::endl;

                // 각 정점의 좌표 출력
                std::cout << "Vertex Positions:" << std::endl;
                for (int i = 0; i < vertexCount; ++i)
                {
                    FbxVector4 position = controlPoints[i];
                    std::cout << "Vertex " << i << ": (" << position[0] << ", " << position[1] << ", " << position[2] << ")" << std::endl;
                }

                // FbxMesh 객체를 얻은 후
                FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();

                if (normalElement)
                {
                    if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // 직접적인(사용자가 입력한) 법선 데이터를 가져옴
                            FbxVector4 normal = normalElement->GetDirectArray().GetAt(vertexIndex);
                            // 이제 normal 변수에는 정점의 법선 정보가 들어있습니다.
                        }
                        else if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // 인덱스를 사용하여 법선 데이터를 가져옴
                            int normalIndex = normalElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);
                            // 이제 normal 변수에는 정점의 법선 정보가 들어있습니다.
                        }
                    }
                }

                // FbxMesh 객체를 얻은 후
                FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();

                if (tangentElement)
                {
                    if (tangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // 직접적인(사용자가 입력한) 접선 데이터를 가져옴
                            FbxVector4 tangent = tangentElement->GetDirectArray().GetAt(vertexIndex);
                            // 이제 tangent 변수에는 정점의 접선 정보가 들어있습니다.
                        }
                        else if (tangentElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // 인덱스를 사용하여 접선 데이터를 가져옴
                            int tangentIndex = tangentElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector4 tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
                            // 이제 tangent 변수에는 정점의 접선 정보가 들어있습니다.
                        }
                    }
                }


                // FbxMesh 객체를 얻은 후
                FbxGeometryElementUV* uvElement = mesh->GetElementUV();

                if (uvElement)
                {
                    if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // 직접적인(사용자가 입력한) UV 데이터를 가져옴
                            FbxVector2 uv = uvElement->GetDirectArray().GetAt(vertexIndex);
                            // 이제 uv 변수에는 정점의 UV 좌표가 들어있습니다.
                        }
                        else if (uvElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // 인덱스를 사용하여 UV 데이터를 가져옴
                            int uvIndex = uvElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
                            // 이제 uv 변수에는 정점의 UV 좌표가 들어있습니다.
                        }
                    }
                }


            }
        }
    }

    // 자식 노드로 재귀 호출
    int childCount = pNode->GetChildCount();
    for (int i = 0; i < childCount; ++i)
    {
        ExtractMeshInfo(pNode->GetChild(i));
    }
}

void PrintNodeInfo(FbxNode* pNode)
{
    if (!pNode)
        return;

    std::cout << "Node Name: " << pNode->GetName() << std::endl;

    // 메시 정보 추출
    ExtractMeshInfo(pNode);

    // 자식 노드로 재귀 호출
    int childCount = pNode->GetChildCount();
    for (int i = 0; i < childCount; ++i)
    {
        PrintNodeInfo(pNode->GetChild(i));
    }
}

int main()
{
    // FBX SDK 초기화
    FbxManager* manager = FbxManager::Create();
    FbxScene* scene = FbxScene::Create(manager, "MyScene");

    // FBX 파일 로드
    FbxImporter* importer = FbxImporter::Create(manager, "");
    importer->Initialize("teapot.fbx", -1, manager->GetIOSettings());
    importer->Import(scene);
    importer->Destroy();

    // 루트 노드부터 시작하여 모든 노드 정보 출력
    FbxNode* rootNode = scene->GetRootNode();
    PrintNodeInfo(rootNode);

    // FBX SDK 정리
    manager->Destroy();

    return 0;
}
