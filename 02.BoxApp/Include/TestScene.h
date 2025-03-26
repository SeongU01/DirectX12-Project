#pragma once
#include "Scene.h"
#include "UploadBuffer.h"

class GameManager;
struct ObjectConstants;
class D12Renderer;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;

class TestScene : public Scene
{
  private:
    TestScene() = default;
    virtual ~TestScene() = default;

  public:
    // Scene을(를) 통해 상속됨

    void Start() override;
    int  Update(const float& dt) override;
    int  LateUpdate(const float& dt) override;
    bool Initialize() override;
    void Render() override;
    void Free() override;

  private:
    bool InitImGui(HWND hWnd);
    void ImGuiRender();

  private:
    void CreateGeometry();
    void CreateShaderAndInputLayout();
    void CreateConstantBuffer();
    void CreateDescriptorHeap();
    void CreateRootSignature();
    void CreatePSO();
  public:
    static TestScene* Create(HWND hWnd);
    bool              _init = false;

  private:
    GameManager*               _pGameManager = nullptr;
    ID3D12GraphicsCommandList* _cmdList = nullptr;
    D12Renderer*               _renderer = nullptr;

  private:
    MeshGeometry*                         _meshes = nullptr;
    ComPtr<ID3DBlob>                      _vsCode = nullptr;
    ComPtr<ID3DBlob>                      _psCode = nullptr;
    std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;
    UploadBuffer<ObjectConstants>*        _objectCB = nullptr;
    ComPtr<ID3D12RootSignature>           _rootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap>          _cbvHeap = nullptr;
    ID3D12PipelineState*                  _pso = nullptr;
    float                                 _phi = XM_PIDIV4;
    float                                 _radius = 5.f;
    float                                 _theta = 1.5f * XM_PI;
    POINT                                 _lastMousePos;

    XMFLOAT4X4 _world = Matrix::Identity;
    XMFLOAT4X4 _view = Matrix::Identity;
    XMFLOAT4X4 _proj = Matrix::Identity;
};
