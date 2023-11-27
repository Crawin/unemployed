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
                // �޽� ���� ���� ����
                int vertexCount = mesh->GetControlPointsCount();
                FbxVector4* controlPoints = mesh->GetControlPoints();

                std::cout << "Mesh Information:" << std::endl;
                std::cout << "Vertex Count: " << vertexCount << std::endl;

                // �� ������ ��ǥ ���
                std::cout << "Vertex Positions:" << std::endl;
                for (int i = 0; i < vertexCount; ++i)
                {
                    FbxVector4 position = controlPoints[i];
                    std::cout << "Vertex " << i << ": (" << position[0] << ", " << position[1] << ", " << position[2] << ")" << std::endl;
                }

                // FbxMesh ��ü�� ���� ��
                FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();

                if (normalElement)
                {
                    if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // ��������(����ڰ� �Է���) ���� �����͸� ������
                            FbxVector4 normal = normalElement->GetDirectArray().GetAt(vertexIndex);
                            // ���� normal �������� ������ ���� ������ ����ֽ��ϴ�.
                        }
                        else if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // �ε����� ����Ͽ� ���� �����͸� ������
                            int normalIndex = normalElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);
                            // ���� normal �������� ������ ���� ������ ����ֽ��ϴ�.
                        }
                    }
                }

                // FbxMesh ��ü�� ���� ��
                FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();

                if (tangentElement)
                {
                    if (tangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // ��������(����ڰ� �Է���) ���� �����͸� ������
                            FbxVector4 tangent = tangentElement->GetDirectArray().GetAt(vertexIndex);
                            // ���� tangent �������� ������ ���� ������ ����ֽ��ϴ�.
                        }
                        else if (tangentElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // �ε����� ����Ͽ� ���� �����͸� ������
                            int tangentIndex = tangentElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector4 tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
                            // ���� tangent �������� ������ ���� ������ ����ֽ��ϴ�.
                        }
                    }
                }


                // FbxMesh ��ü�� ���� ��
                FbxGeometryElementUV* uvElement = mesh->GetElementUV();

                if (uvElement)
                {
                    if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            // ��������(����ڰ� �Է���) UV �����͸� ������
                            FbxVector2 uv = uvElement->GetDirectArray().GetAt(vertexIndex);
                            // ���� uv �������� ������ UV ��ǥ�� ����ֽ��ϴ�.
                        }
                        else if (uvElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            // �ε����� ����Ͽ� UV �����͸� ������
                            int uvIndex = uvElement->GetIndexArray().GetAt(vertexIndex);
                            FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
                            // ���� uv �������� ������ UV ��ǥ�� ����ֽ��ϴ�.
                        }
                    }
                }


            }
        }
    }

    // �ڽ� ���� ��� ȣ��
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

    // �޽� ���� ����
    ExtractMeshInfo(pNode);

    // �ڽ� ���� ��� ȣ��
    int childCount = pNode->GetChildCount();
    for (int i = 0; i < childCount; ++i)
    {
        PrintNodeInfo(pNode->GetChild(i));
    }
}

int main()
{
    // FBX SDK �ʱ�ȭ
    FbxManager* manager = FbxManager::Create();
    FbxScene* scene = FbxScene::Create(manager, "MyScene");

    // FBX ���� �ε�
    FbxImporter* importer = FbxImporter::Create(manager, "");
    importer->Initialize("teapot.fbx", -1, manager->GetIOSettings());
    importer->Import(scene);
    importer->Destroy();

    // ��Ʈ ������ �����Ͽ� ��� ��� ���� ���
    FbxNode* rootNode = scene->GetRootNode();
    PrintNodeInfo(rootNode);

    // FBX SDK ����
    manager->Destroy();

    return 0;
}
