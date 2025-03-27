#include "D12Renderer.h"
#include "GameManager.h"
#include "GameTimer.h"
using namespace Microsoft::WRL;
bool D12Renderer::Initialize()
{
  _clientWidth = WINCX;
  _clientHeight = WINCY;
#if defined(DEBUG) || defined(_DEBUG)
  ComPtr<ID3D12Debug> debugController;
  ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
  debugController->EnableDebugLayer();
#endif
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));

  // device 생성
  HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
                                             IID_PPV_ARGS(&_device));
  if (FAILED(hardwareResult))
  {
    ComPtr<IDXGIAdapter> pWarpAdapter;
    ThrowIfFailed(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

    ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&_device)));
  }

  // fence 생성 및 Descriptor 크기 계산
  ThrowIfFailed(
      _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
  _rtvDescriptorSize =
      _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  _dsvDescriptorSize =
      _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  _cbvSrvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // 4X MSAA 품질 수준 지원 점검
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
  msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  msQualityLevels.SampleCount = 4;
  msQualityLevels.Format = _backBufferFormat;
  msQualityLevels.NumQualityLevels = 0;
  ThrowIfFailed(
      _device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                   &msQualityLevels, sizeof(msQualityLevels)));
  _4xMsaaQuality = msQualityLevels.NumQualityLevels;
  assert(_4xMsaaQuality > 0 && "Unexpected MSAA quality level");

  // Command Queue 생성
  CreateCommandObjects();
  CreateSwapChain();
  CreateRtvAndDsvDescriptorHeaps();
  return true;
}

void D12Renderer::OnResize()
{
  assert(_device);
  assert(_swapChain);
  assert(_directCommandAllocator);
  _clientWidth = WINCX;
  _clientHeight = WINCY;
  FlushCommandQueue();
  ThrowIfFailed(_commandList->Reset(_directCommandAllocator.Get(), nullptr));

  for (size_t i = 0; i < SwapChainBufferCount; i++)
  {
    _swapChainBuffer[i].Reset();
  }
  _depthStencilBuffer.Reset();
  ThrowIfFailed(_swapChain->ResizeBuffers(
      SwapChainBufferCount, _clientWidth, _clientHeight, _backBufferFormat,
      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
  _currentBackBuffer = 0;
  // rtv 생성
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
      _rtvHeap->GetCPUDescriptorHandleForHeapStart());
  for (size_t i = 0; i < SwapChainBufferCount; i++)
  {
    ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_swapChainBuffer[i])));
    _device->CreateRenderTargetView(_swapChainBuffer[i].Get(), nullptr,
                                    rtvHeapHandle);
    rtvHeapHandle.Offset(1, _rtvDescriptorSize);
  }
  // dsv 생성
  D3D12_RESOURCE_DESC depthStencilDesc;
  depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  depthStencilDesc.Alignment = 0;
  depthStencilDesc.Width = _clientWidth;
  depthStencilDesc.Height = _clientHeight;
  depthStencilDesc.DepthOrArraySize = 1;
  depthStencilDesc.MipLevels = 1;
  depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  depthStencilDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
  depthStencilDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
  depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  D3D12_CLEAR_VALUE optClear;
  optClear.Format = _depthStencilFormat;
  optClear.DepthStencil.Depth = 1.f;
  optClear.DepthStencil.Stencil = 0;
  CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
  ThrowIfFailed(_device->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
      D3D12_RESOURCE_STATE_COMMON, &optClear,
      IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())));

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
  dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Format = _depthStencilFormat;
  dsvDesc.Texture2D.MipSlice = 0;
  _device->CreateDepthStencilView(_depthStencilBuffer.Get(), &dsvDesc,
                                     DepthStencilView());

  // depth 버퍼를 initial 상태로 만들어주기
  CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      _depthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
      D3D12_RESOURCE_STATE_DEPTH_WRITE);
  // 커맨드 등록
  _commandList->ResourceBarrier(1, &barrier);
  // 커맨드 실행
  ThrowIfFailed(_commandList->Close());
  ID3D12CommandList* cmdList[] = {_commandList.Get()};
  _commandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
  // GPU동기화
  FlushCommandQueue();
  // 뷰포트.
  _screenViewport.TopLeftX = 0;
  _screenViewport.TopLeftY = 0;
  _screenViewport.Width = static_cast<float>(_clientWidth);
  _screenViewport.Height = static_cast<float>(_clientHeight);
  _screenViewport.MinDepth = 0.0f;
  _screenViewport.MaxDepth = 1.0f;

  _scissorRect = {0, 0, _clientWidth, _clientHeight};
}

void D12Renderer::ResetCommandList()
{
  ThrowIfFailed(_commandList->Reset(_directCommandAllocator.Get(), nullptr));
}

void D12Renderer::FlushCommandQueue()
{
  _fenceValue++;
  ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _fenceValue));
  // GPU가 fence 지점까지 커맨드를 처리할 때 까지 기다림(동기화)
  if (_fence->GetCompletedValue() < _fenceValue)
  {
    HANDLE eventHandle =
        CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
    ThrowIfFailed(_fence->SetEventOnCompletion(_fenceValue, eventHandle));
    WaitForSingleObject(eventHandle, INFINITE);
    CloseHandle(eventHandle);
  }
}

void D12Renderer::BeginDraw()
{

}

void D12Renderer::EndDraw()
{

}

ID3D12Resource* D12Renderer::CurrentBackBuffer()
{
  return _swapChainBuffer[_currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D12Renderer::CurrentBackBufferView()
{
  return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      _rtvHeap->GetCPUDescriptorHandleForHeapStart(), _currentBackBuffer,
      _rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D12Renderer::DepthStencilView()
{
  return _dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void D12Renderer::Free()
{

}

void D12Renderer::CalculateFrameRate()
{
  static int frameCnt = 0;
  static float elapsed = 0.f;
  frameCnt++;
  if (GameTimer::GetInstance()->TotalTime() - elapsed >= 1.f)
  {
    float fps = (float)frameCnt;
    float mspf = 1000.f / fps;
    std::wstring fpsStr = std::to_wstring(fps);
    std::wstring mspfStr = std::to_wstring(mspf);
    std::wstring windowText =
         L"    fps: " + fpsStr + L"   mspf: " + mspfStr;

    SetWindowText(GameManager::GetInstance()->GetWindow(), windowText.c_str());

    // Reset for next average.
    frameCnt = 0;
    elapsed+= 1.0f;
  }
}

void D12Renderer::ChangeBackBuffer() 
{
    _currentBackBuffer = (_currentBackBuffer + 1) % SwapChainBufferCount;
}

float D12Renderer::AspectRatio()
{
    return static_cast<float>(_clientWidth) / _clientHeight;
}

void D12Renderer::CreateCommandObjects()
{
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ThrowIfFailed(
      _device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));
  ThrowIfFailed(_device->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_directCommandAllocator.GetAddressOf())));
  ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           _directCommandAllocator.Get(), nullptr,
                                           IID_PPV_ARGS(_commandList.GetAddressOf())));
  _commandList->Close();
}

void D12Renderer::CreateSwapChain()
{
  _swapChain.Reset();
  DXGI_SWAP_CHAIN_DESC sd;
  sd.BufferDesc.Width = _clientWidth;
  sd.BufferDesc.Height = _clientHeight;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.Format = _backBufferFormat;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.SampleDesc.Count = _4xMsaaState ? 4 : 1;
  sd.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 후면 버퍼의 속성
  sd.BufferCount = SwapChainBufferCount;
  sd.OutputWindow = GameManager::GetInstance()->GetWindow();
  sd.Windowed = true;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  ThrowIfFailed(_dxgiFactory->CreateSwapChain(_commandQueue.Get(), &sd,
                                              _swapChain.GetAddressOf()));
}

void D12Renderer::CreateRtvAndDsvDescriptorHeaps()
{
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
  rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvHeapDesc.NodeMask = 0;
  ThrowIfFailed(_device->CreateDescriptorHeap(
      &rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf())));
  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  dsvHeapDesc.NodeMask = 0;
  ThrowIfFailed(_device->CreateDescriptorHeap(
      &dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf())));
}

ComPtr<ID3D12Resource> D12Renderer::CreateDefaultBuffer(
    const void* initData, uint64_t byteSize,
    ComPtr<ID3D12Resource>& updloadBuff)
{
  return d3dUtil::CreateDefaultBuffer(_device.Get(), _commandList.Get(),
                                      initData, byteSize, updloadBuff);
}

