#pragma once

// dx12 ·»´õ·¯



class Renderer
{
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

private:
	Renderer() {}
	~Renderer() {}

	bool CreateDevice();
	bool CreateSwapChain();
	bool CreateCommandQueueAndList();

public:
	bool Init();

private:
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
};

