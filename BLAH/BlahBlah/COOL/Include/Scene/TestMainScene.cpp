#include "framework.h"
#include "TestMainScene.h"
#include "ResourceManager.h"
#include "ECS/Component.h"
#include "ECS/ECSManager.h"
#include "Shader/Shader.h"

bool TestMainScene::LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    // load Sky material
    m_SkyMaterialIdx = m_ResourceManager->GetMaterial("_SkyRender", commandList);
    if (m_SkyMaterialIdx == -1) return false;

    return true;
}

void TestMainScene::OnPreRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// 추가된 부분
	//m_ResourceManager->SetPSO(commandList, m_SkyMaterialIdx);
	auto* skyMaterial = m_ResourceManager->GetMaterial(m_SkyMaterialIdx);
	skyMaterial->GetShader()->SetPipelineState(commandList);

	// set light data idx
	//skyMaterial->SetDataIndex(0, m_ResourceManager->m_LightIdx);
	// todo mainlight를 찾아야 함
	//skyMaterial->SetDataIndex(1, 0);
	//skyMaterial->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

	int lightResIdx = m_ResourceManager->m_LightIdx;
	int mainLightIdx = 0;

	std::function<void(component::DayLight*)> func =
		[commandList, lightResIdx, mainLightIdx](component::DayLight* dayLight) {
		XMFLOAT4 noonLight = dayLight->GetNoonLight();
		XMFLOAT4 moonLight = dayLight->GetMoonLight();
		XMFLOAT4 sunsetLight = dayLight->GetSunSetLight();
		float angle = dayLight->GetLightAngle();

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &noonLight, static_cast<int>(DAYLIGHT_ROOTCONST::DAY_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &moonLight, static_cast<int>(DAYLIGHT_ROOTCONST::MOON_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &sunsetLight, static_cast<int>(DAYLIGHT_ROOTCONST::SUNSET_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &lightResIdx, static_cast<int>(DAYLIGHT_ROOTCONST::LIGHT_RESOURCE_INDEX));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &mainLightIdx, static_cast<int>(DAYLIGHT_ROOTCONST::MAIN_LIGHT_INDEX));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &angle, static_cast<int>(DAYLIGHT_ROOTCONST::LIGHT_ANGLE));

		commandList->DrawInstanced(6, 1, 0, 0);
		};

	m_ECSManager->Execute(func);

	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

bool TestMainScene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    Scene::Enter(commandList);
    return true;
}

void TestMainScene::Update(float deltaTime)
{
    Scene::Update(deltaTime);
}

void TestMainScene::Exit()
{
}

bool TestMainScene::ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return false;
}

//void TestMainScene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
//{
//	// heap set
//	SetResourceHeap(commandLists[0]);
//
//	// animate first
//	AnimateToSO(commandLists[0]);
//
//	// mrt render end
//	RenderOnMRT(commandLists[0], resultDsv);
//
//
//	
//
//	///////////////////////////////////////////////////////////////////////////////////////////
//
//	// update light, and shadowm map 
//	UpdateLightData(commandLists[0]);
//
//	// post processing
//	PostProcessing(commandLists[0], resultRtv, resultDsv);
//}
