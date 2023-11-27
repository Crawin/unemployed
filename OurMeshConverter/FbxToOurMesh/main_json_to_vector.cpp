#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ParseJson(const char* jsonFileName, std::vector<float>& vertices, std::vector<float>& normals,
    std::vector<float>& uv, std::vector<float>& tangents, std::vector<int>& indices) {
    std::ifstream inputFile(jsonFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFileName << std::endl;
        return;
    }

    try {
        json jsonData;
        inputFile >> jsonData;

        // "meshes" �迭�� ��ȸ
        for (const auto& mesh : jsonData["meshes"]) {
            // "vertices" �迭�� �о vertices�� �߰�
            for (const auto& vertex : mesh["vertices"]) {
                vertices.push_back(vertex[0]);
                vertices.push_back(vertex[1]);
                vertices.push_back(vertex[2]);
            }

            // "normals" �迭�� �о normals�� �߰�
            for (const auto& normal : mesh["normals"]) {
                normals.push_back(normal[0]);
                normals.push_back(normal[1]);
                normals.push_back(normal[2]);
            }

            // "uv" �迭�� �о uv�� �߰�
            for (const auto& uvCoord : mesh["uv"]) {
                uv.push_back(uvCoord[0]);
                uv.push_back(uvCoord[1]);
            }

            // "tangents" �迭�� �о tangents�� �߰�
            for (const auto& tangent : mesh["tangents"]) {
                tangents.push_back(tangent[0]);
                tangents.push_back(tangent[1]);
                tangents.push_back(tangent[2]);
            }

            // "indices" �迭�� �о indices�� �߰�
            for (const auto& index : mesh["indices"]) {
                indices.push_back(index);
            }
        }
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    inputFile.close();
}

int main() {
    std::vector<float> vertices, normals, uv, tangents;
    std::vector<int> indices;

    const char* jsonFileName = "your_parsed_data.json";

    ParseJson(jsonFileName, vertices, normals, uv, tangents, indices);

    // ���� ������ �����͸� ����� �� �ֽ��ϴ�.
    // vertices, normals, uv, tangents, indices���� JSON���� �о�� ������ ��� �ֽ��ϴ�.

    return 0;
}
