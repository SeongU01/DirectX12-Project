#include "TestScene.h"
#include "Client_Define.h"
#include "GameObject.h"
#include "Camera.h"
#include "Transform.h"

float _NEAR = 0.01f;
float _FAR = 100000.f;
float _FOV = DirectX::XM_PIDIV2;

void TestScene::Free()
{
	//ImGui_ImplDX11_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();
}

bool TestScene::InitImGui(HWND hWnd)
{
	//imGui 초기화
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();

	//setup dear imgui style
	//ImGui::StyleColorsDark();

	//setup platform/render backends
	//ImGui_ImplWin32_Init(hWnd);
	//ImGui_ImplDX12_Init(_device.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT, _srvHeap.Get(), _srvHeap->GetCPUDescriptorHandleForHeapStart(), _srvHeap->GetGPUDescriptorHandleForHeapStart());
	return true;
}

void TestScene::ImGuiRender()
{
	//imgui render 코드
	//ImGui::Render();
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),//command List);
}


void TestScene::Start()
{

}

int TestScene::Update(const float& dt)
{

	return 0;
}

int TestScene::LateUpdate(const float& dt)
{
	

	return 0;
}

bool TestScene::Initialize()
{
	_pGameManager = GameManager::GetInstance();

	return true;
}

void TestScene::Render()
{
	
}

TestScene* TestScene::Create(HWND hWnd)
{
	TestScene* pInstance = new TestScene;
	pInstance->InitImGui(hWnd);
	return pInstance;
}
