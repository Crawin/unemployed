#include "framework.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "Renderer/Renderer.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

bool ShaderManager::LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName)
{
	//std::ifstream shaderFile(fileName, std::ios::binary);

	//if (shaderFile.fail()) {
	//	DebugPrint(std::format("Failed to open shader file!! fileName: {}", fileName));
	//	return false;
	//}

	std::shared_ptr<Shader> shader = std::make_shared<Shader>();
	if (false == Renderer::GetInstance().CreateShader(commandList, fileName, shader)) {
		DebugPrint(std::format("Failed to load shader file!! fileName: {}", fileName));
		return false;
	}

	m_Shaders.push_back(shader);

	DebugPrint(std::format("loaded file name: {}", shader->m_Name));
	
	return true;
}

bool ShaderManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	DebugPrint(std::format("Shader Directory ::::::: {}", pathName));

	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		if (file.is_directory()) continue;

		std::string fileName = pathName + file.path().filename().string();
		if (LoadFile(commandList, fileName) == false) return false;
	}

	DebugPrint("\n");

	return true;
}

std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& name)
{
	for (auto& shader : m_Shaders)
		if (shader->GetName() == name)
			return shader;
	
	// todo
	// 없으면 동적으로 로딩 시키게 할지 고민해보는 시간을 가져보는것은 어떨지 생각해보는 것을 고려해보자

	return nullptr;
}
