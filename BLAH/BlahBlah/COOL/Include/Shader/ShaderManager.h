#pragma once

class Shader;
class ShaderManager
{
	std::vector<std::shared_ptr<Shader>> m_Shaders;

public:
	ShaderManager();
	~ShaderManager();

private:
	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName);

public:
	bool LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName);

	std::shared_ptr<Shader> GetShader(const std::string& name);
};

