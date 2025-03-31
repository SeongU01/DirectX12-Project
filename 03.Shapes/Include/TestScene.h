#pragma once
#include "FrameResrouce.h"
#include "Scene.h"
#include "UploadBuffer.h"

class GameManager;
class D12Renderer;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;
struct RenderItem
{
    RenderItem() = default;
    XMFLOAT4X4               world = Matrix::Identity;
    int                      numFrameDirtyFlag = 3;
    UINT                     objCbIndex = 1;
    MeshGeometry*            geo = nullptr;
    D3D12_PRIMITIVE_TOPOLOGY primitiveType =
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    UINT baseVertexLocation = 0;
};

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
    void UpdateCamera(float dt);

    Vector3 _eyePos = {0.f, 0.f, 0.f};
    Matrix  _view = Matrix::Identity;
    Matrix  _proj = Matrix::Identity;

    float _theta = 1.5f * XM_PI;
    float _phi = 0.2f * XM_PI;
    float _radius = 15.0f;

  public:
    static TestScene* Create(HWND hWnd);
    bool              _init = false;
    bool              isWireFrame = false;

  private:
    GameManager*                                        _pGameManager = nullptr;
    ID3D12GraphicsCommandList*                          _cmdList = nullptr;
    D12Renderer*                                        _renderer = nullptr;
    std::unordered_map<std::string, MeshGeometry*>      _geometries;
    std::unordered_map<std::string, ComPtr<ID3D10Blob>> _shaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> _psos;
    std::vector<D3D12_INPUT_ELEMENT_DESC>                        _inputLayout;
    std::vector<RenderItem*> _allRenderItems;
    std::vector<RenderItem*> _opaqueItems;

    UINT                        _passCBVOffset = 0;
    PassConstants               _mainPassCB;
    ID3D12DescriptorHeap*       _CBVHeap = nullptr;
    std::vector<FrameResource*> _frameResources;
    FrameResource*              _currentFrame = nullptr;
    int                         _currentFrameIndex = 0;

    ID3D12RootSignature* _rootSignature = nullptr;

  public:
    void CreateShapeGeometry();
    void CreateRenderItems();
    void CreateDescriptorHeaps();
    void CreateConstantBufferViews();
    void CreateFrameResource();
    void CreateRootSignature();
    void CreatePSOs();
    void CreateInputLayoutAndShader();
    void DrawRenderItem(ID3D12GraphicsCommandList*      cmdList,
                        const std::vector<RenderItem*>& items);

    void UpdateMainPassCB(const float& dt);
    void UpdateObjectCBs(const float& dt);
};
