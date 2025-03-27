#pragma once
#include "Scene.h"
#include "UploadBuffer.h"

class GameManager;
struct ObjectConstants;
class D12Renderer;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;

const int NUMFRAMERESOURCE = 3;

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

  public:
    static TestScene* Create(HWND hWnd);
    bool              _init = false;

  private:
    GameManager*                                   _pGameManager = nullptr;
    ID3D12GraphicsCommandList*                     _cmdList = nullptr;
    D12Renderer*                                   _renderer = nullptr;
    std::unordered_map<std::string, MeshGeometry*> _geometries;
    std::unordered_map<std::string, ID3D10Blob*>   _shaders;
    std::unordered_map<std::string, ID3D12PipelineState*> _psos;
    std::vector<RenderItem*>                              _allRenderItems;
    std::vector<RenderItem*>                              _opaqueItems;

  public:
    void CreateShapeGeometry();
    void CreateRenderItems();
};
