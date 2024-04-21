#include "Scene.h"
//#include "Renderer/Renderer.h"
#include "framework.h"
#include "ECS/ECSManager.h"
#include "ResourceManager.h"
#include "ECS/Component.h"
#include "ECS/ECS_System.h"
#include "Shader/Shader.h"

#ifdef _DEBUG
#include "App/InputManager.h"
#endif
//#define SCENE_PATH "SceneData\\Scene\\"

Scene::Scene()
{
	m_ResourceManager = new ResourceManager;
}

Scene::~Scene()
{
	if (m_ResourceManager) delete m_ResourceManager;
}


bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	m_ECSManager = std::make_shared<ECSManager>();

	m_ResourceManager->SetECSManager(m_ECSManager);

	CHECK_CREATE_FAILED(AddSystem(), std::format("Error on add system!!, name: {}", sceneName));

	CHECK_CREATE_FAILED(m_ResourceManager->Init(commandList, sceneName), std::format("Can't Load Scene, name: {}", sceneName));
	
	CHECK_CREATE_FAILED(LoadSceneExtra(commandList), std::format("Can't Load Scene Extra, name: {}", sceneName));

	m_ECSManager->InitSystem();

	return true;
}

bool Scene::AddSystem()
{
	// default systems
	// 넣는 순서에 따라 system이 돌아가는게 달라짐
	// 우선순위들 정리
	// 1. 각 객체들의 transform을 바꾸는 system
	// 998. todo 충돌처리
	// 999. 부모에 따라 transform을 바꾸는 system(LocalToWorldTransform)
	m_ECSManager->InsertSystem(new ECSsystem::SyncPosition);
	m_ECSManager->InsertSystem(new ECSsystem::AnimationPlayTimeAdd);
	m_ECSManager->InsertSystem(new ECSsystem::SyncWithTransform);
	m_ECSManager->InsertSystem(new ECSsystem::DayLight);
	m_ECSManager->InsertSystem(new ECSsystem::UpdateInput);
	m_ECSManager->InsertSystem(new ECSsystem::SimulatePhysics);
	//m_ECSManager->InsertSystem(new ECSsystem::SendToServer);
	m_ECSManager->InsertSystem(new ECSsystem::ChangeAnimationTest);

	m_ECSManager->InsertSystem(new ECSsystem::LocalToWorldTransform);

	m_ECSManager->InsertSystem(new ECSsystem::CollideCkeck);


	return true;
}

bool Scene::LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	//m_ResourceManager->GetMaterial("", commandList);

	return true;
}

void Scene::SetResourceHeap(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_ResourceManager->SetResourceHeap(commandList);

}

void Scene::AnimateToSO(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	ResourceManager* manager = m_ResourceManager;

	// Set Animation PSO
	m_ResourceManager->GetAnimationShader()->SetPipelineState(commandList);

	// animate and set animed data
	std::function<void(component::Renderer*, component::AnimationExecutor*)> animate = [&commandList, manager](component::Renderer* renderComponent, component::AnimationExecutor* executor) {
		int meshIdx = renderComponent->GetMesh();
		Mesh* mesh = manager->GetMesh(meshIdx);
		if (mesh && mesh->IsSkinned() && mesh->GetVertexNum() > 0) {
			// Stream Out data set to  Stream Out
			int bufidx = executor->GetStreamOutBuffer();
			manager->SetResourceState(commandList, RESOURCE_TYPES::VERTEX, bufidx, D3D12_RESOURCE_STATE_STREAM_OUT);
			auto resPtr = executor->GetStreamOutBuffer();

			// should set stream buf pos to zero
			*(executor->m_StreamSize) = 0;

			// SO set
			const auto& view = executor->GetStreamOutBufferView();
			D3D12_STREAM_OUTPUT_BUFFER_VIEW bufView[] = { view };
			commandList->SOSetTargets(0, _countof(bufView), bufView);

			// set bone here
			int boneIdx = manager->GetBone(mesh->GetBoneIdx())->GetBoneDataIdx();
			commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &boneIdx, static_cast<int>(ANIM_ROOTCONST::BONE_INDEX));

			// set animation here
			executor->SetData(commandList, manager);

			// animate here
			mesh->SetVertexBuffer(commandList);
			mesh->Animate(commandList);

			// 완료 후 Vertex Buffer로 변경
			manager->SetResourceState(commandList, RESOURCE_TYPES::VERTEX, bufidx, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
		};

	m_ECSManager->Execute(animate);

	//std::function<void(component::Animation*)> debug = [](component::Animation* anim) {

	//	DebugPrint(std::format("{}", *(anim->m_StreamSize)));
	//	};

	//m_ECSManager->Execute(debug);
}

void Scene::RenderOnMRT(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	auto& res = m_ResourceManager;

	// default deffered renderer
	m_ResourceManager->SetMRTStates(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// clear mrt
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ResourceManager->ClearMRTS(commandList, clearColor);

	// OM set
	auto rtMRT = m_ResourceManager->GetDefferedRenderTargetStart();
	commandList->OMSetRenderTargets(static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END), &rtMRT, true, &resultDsv);
	//commandLists[0]->OMSetRenderTargets(1, &resultRtv, true, &resultDsv);

	// get Camera
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&commandList, &camVec](component::Camera* cam) {	camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	camVec[0]->SetCameraData(commandList);

	OnPreRender(commandList, resultDsv);


	BoundingFrustum& cameraFustum = camVec[0]->GetBoundingFrustum();
	BoundingOrientedBox tempOBB;

	// make function
	std::function<void(component::Renderer*)> func = [&commandList, &res, &cameraFustum, &tempOBB](component::Renderer* renderComponent) {
		int materialIdx = renderComponent->GetMaterial();
		int meshIdx = renderComponent->GetMesh();

		Material* material = res->GetMaterial(materialIdx);
		Mesh* mesh = res->GetMesh(meshIdx);

		const BoundingOrientedBox& meshOBB = mesh->GetBoundingBox();
		meshOBB.Transform(tempOBB, XMLoadFloat4x4(&renderComponent->GetWorldMatrix()));

		// frustum culling
		if (cameraFustum.Intersects(tempOBB) == false) return;


		material->GetShader()->SetPipelineState(commandList);
		material->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		//XMFLOAT4X4 t = Matrix4x4::Identity();
		//res->m_Meshes[mesh]->SetVertexBuffer(commandLists[0]);

		const auto& view = renderComponent->GetVertexBufferView();
		D3D12_VERTEX_BUFFER_VIEW bufView[] = { view };
		commandList->IASetVertexBuffers(0, _countof(bufView), bufView);

		mesh->Render(commandList, renderComponent->GetWorldMatrix());
		};

	// execute function
	m_ECSManager->Execute<component::Renderer>(func);

	OnPostRender(commandList, resultDsv);

	// render sky here

}

void Scene::UpdateLightData(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// render shadow map

	// light map
	// 1. light map을 만드는 모든 light들에 대해 프러스텀 컬링을 실시함 (renderer를 찾아 모아옴)
	// 2. lightmap 상태 변경(tex -> present)
	// 3. OM SET
	// 4. render
	// 4 - 1. shader에 light 데이터가 가야하는데 어케하지? light data를 
	// 4 - 2.
	// 5. present -> tex;



	// for lambda capture
	std::shared_ptr<ECSManager> ecs = m_ECSManager;
	auto& res = m_ResourceManager;


	// set light data to shader
	int count = 0;
	LightData* data = m_ResourceManager->m_MappedLightData;

	// unoccupy
	res->UnOccupyShadowRTVs();

	// get Camera
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&camVec](component::Camera* cam) { camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	int& activeLights = m_ActiveLightSize;
	activeLights = 0;

	std::function<void(component::Light*)> updateLight = [&count, data, res, &camVec, &activeLights](component::Light* lightComp) {

		// todo
		// if 뭐 얘가 좀 쌘 애다, shadowmap 만들기
		LightData& light = lightComp->GetLightData();
		int shadowMapIdx = -1;

		// 일단 light cam의 연결을 끊는다.
		light.m_CameraIdx = -1;

		// if light dir == directional, update light map
		switch (static_cast<LIGHT_TYPES>(light.m_LightType)) {
		case LIGHT_TYPES::DIRECTIONAL_LIGHT: 
		{
			++activeLights;

			// todo 
			// 하드코딩 경고!!!!!!!!!!!!!!!!!!!!!!!!!
			auto& p = camVec[0]->GetPosition();
			
			XMFLOAT3 up = light.m_Direction;
			up.x *= -5000.0;
			up.y *= -5000.0;
			up.z *= -5000.0;

			XMFLOAT3 pos = { p.x + up.x, p.y + up.y, p.z + up.z };
			light.m_Position = pos;

			// 뒷면이라면
			if (up.y < 0) {
				// 셰도우맵핑을 끄자
				break;
			}
			// 해당 함수가 불리면 shadow mapping을 한다.
			shadowMapIdx = res->GetUnOccupiedShadowMapRenderTarget(static_cast<LIGHT_TYPES>(light.m_LightType));
			res->UpdateShadowMapView(shadowMapIdx, light);

			// set result
			light.m_CameraIdx = res->GetShadowMapCamIdx(shadowMapIdx);
			light.m_ShadowMapResults[0] = shadowMapIdx + res->GetShadowMapRTVStartIdx();

			// todo
			// 현재는 shadowmap 0번이 shadowmap 렌더타겟 0번을 그대로 가리키고 있기 때문에
			res->SetShadowMapRTVIdx(shadowMapIdx, shadowMapIdx);

			//DebugPrint(std::format("vie: {}, {}, {}", -mat._14, -mat._24, -mat._34));
			//DebugPrint(std::format("lig: {}, {}, {}", pos.x, pos.y, pos.z));
		}
			break;
		case LIGHT_TYPES::SPOT_LIGHT:
			DebugPrint("No current shadow map setting for spot light now");

			break;
		case LIGHT_TYPES::POINT_LIGHT:
			DebugPrint("No current shadow map setting for point light now");
			break;

		};


		// clear Dsv;
		// todo 
		// 뭐 카메라 거리에 자르거나 할 수 있게 할까?


		memcpy(data + count++, &light, sizeof(LightData));
		};

	m_ECSManager->Execute(updateLight);

	// 여기서 셰도으맵을 만든대


	// 
	// OMSet
	// Render
	// LOOP
	// 
	// set light map pso
	int shadowMat = m_ResourceManager->GetShadowMappingMaterial();
	//m_ResourceManager->SetPSO(commandList, shadowMat);
	Material* mat = m_ResourceManager->GetMaterial(shadowMat);
	mat->GetShader()->SetPipelineState(commandList);


	// Set Resource To RTV
	m_ResourceManager->SetShadowMapStates(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear RTV Clear DSV
	const float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ResourceManager->ClearShadowMaps(commandList, clearColor);
	auto dsv = m_ResourceManager->GetShadowMapDepthStencil();
		//m_ResourceManager->m_ShadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();

	// set viewport
	// 하드코딩 경보
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(8192);
	vp.Height = static_cast<float>(8192);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	RECT scRect = { 0, 0, 8192, 8192 };

	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &scRect);
	
	// normal render code

	BoundingFrustum* cameraFustum;
	BoundingOrientedBox tempOBB;

	std::function<void(component::Renderer*)> render = [commandList, res, &cameraFustum, &tempOBB](component::Renderer* rend) {
		Mesh* mesh = res->GetMesh(rend->GetMesh());

		const BoundingOrientedBox& meshOBB = mesh->GetBoundingBox();
		meshOBB.Transform(tempOBB, XMLoadFloat4x4(&rend->GetWorldMatrix()));

		// frustum culling
		if (cameraFustum->Intersects(tempOBB) == false) return;

		const auto& view = rend->GetVertexBufferView();
		D3D12_VERTEX_BUFFER_VIEW bufView[] = { view };
		commandList->IASetVertexBuffers(0, _countof(bufView), bufView);
		//res->RenderMesh(commandList, rend->GetMesh(), rend->GetWorldMatrix());


		// 

		mesh->Render(commandList, rend->GetWorldMatrix());
		};

	// 모든 활성화된 셰도우맵에 대해서
	for (int i = 0; i < m_ResourceManager->m_ShadowMaps.size(); ++i) {
		if (m_ResourceManager->m_ShadowMapOccupied[i] == false) break;
		int idx = res->GetShadowMapRTVIdx(i);
		D3D12_CPU_DESCRIPTOR_HANDLE targetRtv = res->GetShadowMapRenderTarget(idx);

		if (idx < 0) {
			DebugPrint("ERROR~!!!!");
		}
		
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		commandList->OMSetRenderTargets(1, &targetRtv, true, &dsv);

		// set camera
		res->SetShadowMapCamera(commandList, idx);

		// 일단 임시로 update함

		cameraFustum = res->GetShadowMapFrustum(idx);
		ecs->Execute(render);
	}

	vp.Width = static_cast<float>(1280);
	vp.Height = static_cast<float>(720);

	scRect = { 0, 0, 1280, 720};

	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &scRect);
	// set render target

}

void Scene::PostProcessing(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->OMSetRenderTargets(1, &resultRtv, true, &resultDsv);
	m_ResourceManager->SetMRTStates(commandList, D3D12_RESOURCE_STATE_COMMON);
	m_ResourceManager->SetShadowMapStates(commandList, D3D12_RESOURCE_STATE_COMMON);

	int postMat = m_ResourceManager->GetPostProcessingMaterial();

	Material* mat = m_ResourceManager->GetMaterial(postMat);

	// todo light size
	mat->SetDataIndex(static_cast<int>(MRT_POST_ROOTCONST::LIGHT_SIZE), m_ResourceManager->m_LightSize);
	mat->SetDataIndex(static_cast<int>(MRT_POST_ROOTCONST::LIGHT_IDX), m_ResourceManager->m_LightIdx);
	mat->GetShader()->SetPipelineState(commandList);
	//m_ResourceManager->SetPSO(commandList, postMat);
	mat->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

	commandList->DrawInstanced(6, 1, 0, 0);


#ifdef _DEBUG
	// test code
	// todo 적절한 위치로 옮기도록 하자
	if (InputManager::GetInstance().GetDebugMode()) {
		commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		int debugMat = m_ResourceManager->GetDebuggingMaterial();

		//m_ResourceManager->SetPSO(commandList, debugMat);
		m_ResourceManager->GetMaterial(debugMat)->GetShader()->SetPipelineState(commandList);

		int a[2] = { 0, 0 };
		
		if (GetAsyncKeyState(VK_F9) & 0x8000)
			a[0] = m_ResourceManager->GetShadowMapRTVStartIdx();

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, a, 0);
		//m_ResourceManager->m_Materials[postMat]->SetDatas(commandLists[0], static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		commandList->DrawInstanced(6, 1, 0, 0);
	}
#endif
}

bool Scene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (LoadScene(commandList, m_SceneName) == false)
	{
		ERROR_QUIT(std::format("ERROR!! Scene load error, {}", m_SceneName));
	}

	return true;
}

void Scene::Update(float deltaTime)
{
	m_ECSManager->UpdateSystem(deltaTime);
}

void Scene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// heap set
	SetResourceHeap(commandLists[0]);

	// animate first
	AnimateToSO(commandLists[0]);

	// mrt render end
	RenderOnMRT(commandLists[0], resultDsv);
	///////////////////////////////////////////////////////////////////////////////////////////

	// update light, and shadowm map 
	UpdateLightData(commandLists[0]);

	// post processing
	PostProcessing(commandLists[0], resultRtv, resultDsv);
}
