#pragma once

class CScene
{
private:
	std::string sScene_name;
	Renderer* pRenederer;
public:
	static CScene& Instance() {
		static CScene inst;
		return inst;
	}
	bool Init();
	CScene();
	~CScene();
	void Render();
};

