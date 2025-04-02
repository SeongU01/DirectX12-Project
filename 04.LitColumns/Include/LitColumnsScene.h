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
    XMFLOAT4X4               texTransform = Matrix::Identity;
    int                      numFrameDirtyFlag = 3;
    UINT                     objCbIndex = 1;
    Material*                mat = nullptr;
    MeshGeometry*            geo = nullptr;
    D3D12_PRIMITIVE_TOPOLOGY primitiveType =
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    UINT baseVertexLocation = 0;
};

class LitColumnsScene : public Scene
{
  private:
    LitColumnsScene() = default;
    virtual ~LitColumnsScene() = default;

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
    bool  isWireFrame = false;
  public:
    static LitColumnsScene* Create(HWND hWnd);
    bool              _init = false;

  private:
    std::vector<FrameResource*>                    _frameResource;
    FrameResource*                                 _currFrame;
    int                                            _currFrameIndex = 0;
    UINT                                           _cbvSrvDescriptorSize = 0;
    ComPtr<ID3D12RootSignature>                    _rootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap>                   _srvDescriptorHeap = nullptr;
    std::unordered_map<std::string, MeshGeometry*> _geometries;
    std::unordered_map<std::string, Material*>     _materials;
    std::unordered_map<std::string, Texture*>      _textures;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> _psos;
    std::unordered_map<std::string, ComPtr<ID3D10Blob>> _shaders;
    std::vector<D3D12_INPUT_ELEMENT_DESC>               _inputLayout;
    std::vector<RenderItem*>                            _allRenderItem;
    std::vector<RenderItem*>                            _opaqueItems;
    PassConstants                                       _mainPassCB;

  private:
    void CreateRootSignature();
    void CreateShaderAndInputLayout();
    void CreateShapeGeometry();
    void CreateSkullGeometry();
    void CreateMaterials();
    void CreateRenderItmes();
    void CreateFrameResources();
    void CreatePSOs();
    void CreateSrvDescriptorHeap();

    void DrawRenderItem(ID3D12GraphicsCommandList*      cmdList,
                        const std::vector<RenderItem*>& renderItems);
    void UpdateObjectCB(const float& dt);
    void UpdateMainPassCB(const float& dt);
    void UpdateMaterialCB(const float& dt);
  private:
    GameManager*               _pGameManager = nullptr;
    ID3D12GraphicsCommandList* _cmdList = nullptr;
    D12Renderer*               _renderer = nullptr;
};
