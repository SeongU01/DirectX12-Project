#include "TestScene.h"
#include "Client_Define.h"
#include "D12Renderer.h"
#include "FrameResrouce.h"
#include "GeometryGenerator.h"
float _NEAR = 0.01f;
float _FAR  = 1000.f;
float _FOV  = DirectX::XM_PIDIV2;

void TestScene::Free()
{
    for (auto& e : _geometries)
    {
        if (e.second)
        {
            delete e.second;
        }
    }
}

bool TestScene::InitImGui(HWND hWnd)
{

    return true;
}

void TestScene::ImGuiRender() {}

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
    _renderer     = D12Renderer::GetInstance();
    _cmdList      = _renderer->GetCmdList();
    _renderer->ResetCommandList();

    CreateShapeGeometry();
    ThrowIfFailed(_cmdList->Close());
    ID3D12CommandList* cmdlists[] = {_cmdList};
    _renderer->_commandQueue->ExecuteCommandLists(_countof(cmdlists), cmdlists);
    ISREADYCLIENT = true;
    return true;
}

void TestScene::Render() {}

TestScene* TestScene::Create(HWND hWnd)
{
    TestScene* pInstance = new TestScene;
    pInstance->InitImGui(hWnd);
    return pInstance;
}

void TestScene::CreateShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box    = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
    GeometryGenerator::MeshData grid   = geoGen.CreateGrid(20.f, 30.f, 60, 40);
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 60, 40);
    GeometryGenerator::MeshData cylinder =
        geoGen.CreateCylinder(0.5f, 0.3f, 3.f, 20, 20);
    // 모든 기하 구조를 하나의 버텍스/인덱스 버퍼에 담아 인스턴싱한다.
    UINT boxVertexOffset  = 0;
    UINT gridVertexOffset = static_cast<UINT>(box.vertices.size());
    UINT sphereVertexOffset =
        static_cast<UINT>(grid.vertices.size()) + gridVertexOffset;
    UINT cylinderVertexOffset =
        static_cast<UINT>(sphere.vertices.size()) + sphereVertexOffset;

    UINT boxIndexOffset  = 0;
    UINT gridIndexOffset = static_cast<UINT>(box.indices32.size());
    UINT sphererIndexOffset =
        static_cast<UINT>(grid.indices32.size()) + gridIndexOffset;
    UINT cylinderIndexOffset =
        static_cast<UINT>(sphere.indices32.size()) + sphererIndexOffset;

    SubmeshGeometry boxMesh;
    boxMesh.IndexCount         = static_cast<UINT>(box.indices32.size());
    boxMesh.StartIndexLocation = boxIndexOffset;
    boxMesh.BaseVertexLocation = boxVertexOffset;
    SubmeshGeometry gridMesh;
    gridMesh.IndexCount         = static_cast<UINT>(grid.indices32.size());
    gridMesh.StartIndexLocation = gridIndexOffset;
    gridMesh.BaseVertexLocation = gridVertexOffset;
    SubmeshGeometry sphereMesh;
    sphereMesh.IndexCount         = static_cast<UINT>(sphere.indices32.size());
    sphereMesh.StartIndexLocation = sphererIndexOffset;
    sphereMesh.BaseVertexLocation = sphereVertexOffset;
    SubmeshGeometry cylinderMesh;
    cylinderMesh.IndexCount = static_cast<UINT>(cylinder.indices32.size());
    cylinderMesh.StartIndexLocation = cylinderIndexOffset;
    cylinderMesh.BaseVertexLocation = cylinderVertexOffset;

    unsigned long long totalVertexCount =
        box.vertices.size() + grid.vertices.size() + sphere.vertices.size() +
        cylinder.vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    UINT k = 0;
    for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = box.vertices[i].position;
        vertices[k].color = XMFLOAT4(DirectX::Colors::DarkOliveGreen);
    }

    for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = grid.vertices[i].position;
        vertices[k].color = XMFLOAT4(DirectX::Colors::GreenYellow);
    }

    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = sphere.vertices[i].position;
        vertices[k].color = XMFLOAT4(DirectX::Colors::Crimson);
    }

    for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = cylinder.vertices[i].position;
        vertices[k].color = XMFLOAT4(DirectX::Colors::AliceBlue);
    }

    std::vector<uint16_t> indices;
    indices.insert(indices.end(), std::begin(box.GetIndices16()),
                   std::end(box.GetIndices16()));
    indices.insert(indices.end(), std::begin(grid.GetIndices16()),
                   std::end(grid.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()),
                   std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(cylinder.GetIndices16()),
                   std::end(cylinder.GetIndices16()));

    const UINT vertexBufferByteSize =
        static_cast<UINT>(vertices.size()) * sizeof(Vertex);
    const UINT indexBufferByteSize =
        static_cast<UINT>(indices.size()) * sizeof(uint16_t);

    MeshGeometry* geo = new MeshGeometry();
    geo->Name         = "shapeGeo";
    ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(),
               vertexBufferByteSize);
    ThrowIfFailed(D3DCreateBlob(indexBufferByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(),
               indexBufferByteSize);
    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        _renderer->GetDevice(), _cmdList, vertices.data(), vertexBufferByteSize,
        geo->VertexBufferUploader);
    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        _renderer->GetDevice(), _cmdList, indices.data(), indexBufferByteSize,
        geo->IndexBufferUploader);
    geo->VertexByteStride     = sizeof(Vertex);
    geo->VertexBufferByteSize = vertexBufferByteSize;
    geo->IndexFormat          = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize  = indexBufferByteSize;
    geo->DrawArgs["box"]      = boxMesh;
    geo->DrawArgs["grid"]     = gridMesh;
    geo->DrawArgs["sphere"]   = sphereMesh;
    geo->DrawArgs["cylinder"] = cylinderMesh;

    _geometries[geo->Name] = geo;
}

void TestScene::CreateRenderItems()
{

}
