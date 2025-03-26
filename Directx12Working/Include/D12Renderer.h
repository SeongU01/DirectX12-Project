#pragma once
#include "Base.h"
using namespace Microsoft::WRL;
class D12Renderer : public Base, public SingleTon<D12Renderer>
{
  public:
    D12Renderer() = default;
    virtual ~D12Renderer() = default;

  public:
    bool Initialize();
    void OnResize();
    void ResetCommandList();
    /**
     * @brief wait for GPU sync
     */
    void                        FlushCommandQueue();
    void                        BeginDraw();
    void                        EndDraw();
    ID3D12Resource*             CurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
    ID3D12Device*               GetDevice() { return _device.Get(); }
    ID3D12GraphicsCommandList*  GetCmdList() { return _commandList.Get(); }
    ID3D12CommandAllocator* GetCmdalloc() { return _commandAllocator.Get(); }
    ID3D12CommandQueue*     GetCommandQueue() { return _commandQueue.Get(); }

  public:
    // Base��(��) ���� ��ӵ�
    void  Free() override;
    void  CalculateFrameRate();
    void  ChangeBackBuffer();
    float AspectRatio();
  private:
    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    
  public:
    ComPtr<ID3D12Resource> CreateDefaultBuffer(
        const void* initData, uint64_t byteSize,
        ComPtr<ID3D12Resource>& updloadBuff);

  public:
    int _clientWidth = 800;
    int _clientHeight = 600;

  public:
    ComPtr<IDXGIFactory4>  _dxgiFactory;
    ComPtr<IDXGISwapChain> _swapChain;
    ComPtr<ID3D12Device>   _device;
    // fence
    ComPtr<ID3D12Fence> _fence;
    UINT64              _fenceValue = 0;
    // Command
    ComPtr<ID3D12CommandQueue> _commandQueue;
    //=> �������� commandAllocator ,commandList�� ���� ����ó���� �� �� ����
    ComPtr<ID3D12CommandAllocator>    _commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> _commandList;
    // Descriptor Heap
    ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    ComPtr<ID3D12DescriptorHeap> _dsvHeap;
    // Resrouce
    static const int       SwapChainBufferCount = 2;
    ComPtr<ID3D12Resource> _swapChainBuffer[SwapChainBufferCount];
    ComPtr<ID3D12Resource> _depthStencilBuffer;
    int                    _currentBackBuffer = 0;
    // View
    D3D12_VIEWPORT _screenViewport;
    D3D12_RECT     _scissorRect;

  public:
    bool            _4xMsaaState = false; // 4X MSAA enabled
    UINT            _4xMsaaQuality = 0;   // quality level of 4X MSAA
    UINT            _rtvDescriptorSize = 0;
    UINT            _dsvDescriptorSize = 0;
    UINT            _cbvSrvUavDescriptorSize = 0;
    D3D_DRIVER_TYPE _d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT     _backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT     _depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};
