#include "TestScene.h"
#include "Camera.h"
#include "Client_Define.h"
#include "D12Renderer.h"
#include "GameObject.h"
#include "Transform.h"

float _NEAR = 0.01f;
float _FAR = 100000.f;
float _FOV = DirectX::XM_PIDIV2;

void TestScene::Free()
{
  // ImGui_ImplDX11_Shutdown();
  // ImGui_ImplWin32_Shutdown();
  // ImGui::DestroyContext();
}

bool TestScene::InitImGui(HWND hWnd)
{
  // imGui �ʱ�ȭ
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();

  // setup dear imgui style
  // ImGui::StyleColorsDark();

  // setup platform/render backends
  // ImGui_ImplWin32_Init(hWnd);
  // ImGui_ImplDX12_Init(_device.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM,
  // DXGI_FORMAT_D32_FLOAT, _srvHeap.Get(),
  // _srvHeap->GetCPUDescriptorHandleForHeapStart(),
  // _srvHeap->GetGPUDescriptorHandleForHeapStart());
  return true;
}

void TestScene::ImGuiRender()
{
  // imgui render �ڵ�
  // ImGui::Render();
  // ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),//command List);
}

void TestScene::Start() {}

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
  _renderer = D12Renderer::GetInstance();
  return true;
}

void TestScene::Render()
{
  ThrowIfFailed(_renderer->_directCommandAllocator->Reset());
  // Ŀ�ǵ� ����Ʈ�� �缳���ϸ� �޸� ��Ȱ�� ����
  ThrowIfFailed(_renderer->_commandList->Reset(
      _renderer->_directCommandAllocator.Get(), nullptr));
  // viewPort ����
  _renderer->_commandList->RSSetViewports(1, &_renderer->_screenViewport);
  _renderer->_commandList->RSSetScissorRects(1, &_renderer->_scissorRect);
  // ���� ���� �뺸
  auto backBuffer = _renderer->CurrentBackBufferView();
  auto dsv = _renderer->DepthStencilView();
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
      D3D12_RESOURCE_STATE_RENDER_TARGET);
  _renderer->_commandList->ResourceBarrier(1, &barrier);
  _renderer->_commandList->ClearRenderTargetView(
      backBuffer, DirectX::Colors::LightSteelBlue, 0, nullptr);
  // �Ʒ� ����� �����ϰ� Rect ������ �����ؼ� Rect�� Ŭ�����
  // D3D12_RECT rtRect1;
  // rtRect1.left = 200.f;
  // rtRect1.right = 300.f;
  // rtRect1.top = 200.f;
  // rtRect1.bottom = 300.f;
  // D3D12_RECT rtRect2;
  // rtRect2.left = 400.f;
  // rtRect2.right = 500.f;
  // rtRect2.top = 400.f;
  // rtRect2.bottom = 500.f;
  // D3D12_RECT rtRects[2] = {rtRect1,rtRect2};
  //_commandList->ClearRenderTargetView(
  //     backBuffer, DirectX::Colors::LightSteelBlue, 2, rtRects);

  // TODO : ������ begin Draw���� ���ٰ��� ���� ��� �ʼ�!!
  _renderer->_commandList->ClearDepthStencilView(
      dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0,
      nullptr);
  _renderer->_commandList->OMSetRenderTargets(1, &backBuffer, true, &dsv);
  barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_PRESENT);
  _renderer->_commandList->ResourceBarrier(1, &barrier);
  ThrowIfFailed(_renderer->_commandList->Close());
  ID3D12CommandList* cmdList[] = {_renderer->_commandList.Get()};
  _renderer->_commandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
  _renderer->_swapChain->Present(0, 0);
  _renderer->_currentBackBuffer =
      (_renderer->_currentBackBuffer + 1) % _renderer->SwapChainBufferCount;

  // GPU ����ȭ ��ٸ���
  _renderer->FlushCommandQueue();
}

TestScene* TestScene::Create(HWND hWnd)
{
  TestScene* pInstance = new TestScene;
  pInstance->InitImGui(hWnd);
  return pInstance;
}
