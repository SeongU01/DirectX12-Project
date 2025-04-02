#include "LitColumnsScene.h"
#include "Client_Define.h"
#include "D12Renderer.h"
#include "GeometryGenerator.h"
using namespace DirectX;
float _NEAR                  = 0.01f;
float _FAR                   = 1000.f;
float _FOV                   = XM_PIDIV4;
const int gNumFrameResources = 3;
void LitColumnsScene::Free()
{
    for (auto& e : _geometries)
    {
        if (e.second)
        {
            delete e.second;
        }
    }
    for (auto& e : _materials)
    {
        if (e.second)
        {
            delete e.second;
        }
    }
    for (auto& iter : _allRenderItem)
    {
        if (iter)
        {
            delete iter;
        }
    }
    for (auto& iter : _frameResource)
    {
        if (iter)
        {
            delete iter;
        }
    }
}

bool LitColumnsScene::InitImGui(HWND hWnd)
{

    return true;
}

void LitColumnsScene::ImGuiRender() {}

void LitColumnsScene::UpdateCamera(float dt)
{
    if (Input::IsKeyPress(Input::MouseState::DIM_LB))
    {
        float dx = Input::GetMouseMove(Input::MouseMove::DIM_X);
        float dy = Input::GetMouseMove(Input::MouseMove::DIM_Y);

        _theta += dx * 0.005f;
        _phi += dy * 0.005f;
        _phi = MathHelper::Clamp(_phi, 0.1f, XM_PI - 0.1f);
    }
    else if (Input::IsKeyPress(Input::MouseState::DIM_RB))
    {
        float dx = 0.01f * Input::GetMouseMove(Input::MouseMove::DIM_X);
        float dy = 0.01f * Input::GetMouseMove(Input::MouseMove::DIM_Y);
        _radius += dx - dy;
        _radius = MathHelper::Clamp(_radius, 3.f, 30.f);
    }

    _eyePos.x = _radius * sinf(_phi) * cosf(_theta);
    _eyePos.y = _radius * cosf(_phi);
    _eyePos.z = _radius * sinf(_phi) * sinf(_theta);

    XMVECTOR pos    = XMVectorSet(_eyePos.x, _eyePos.y, _eyePos.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&_view, view);
}

void LitColumnsScene::Start() {}

int LitColumnsScene::Update(const float& dt)
{
    // 투영행렬 업데이트
    if (RESIZEFLAG)
    {
        RESIZEFLAG = false;
        XMMATRIX p = XMMatrixPerspectiveFovLH(_FOV, _renderer->AspectRatio(),
                                              _NEAR, _FAR);
        XMStoreFloat4x4(&_proj, p);
    }
    if (Input::IsKeyPress(DIK_1))
    {
        isWireFrame = true;
    }
    else
    {
        isWireFrame = false;
    }
    UpdateCamera(dt);
    _currFrameIndex = (_currFrameIndex + 1) % gNumFrameResources;
    _currFrame      = _frameResource[_currFrameIndex];

    if (_currFrame->Fence != 0 && _renderer->_fence->GetCompletedValue() < _currFrame->Fence)
    {
        HANDLE eventHandle =
            CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(_renderer->_fence->SetEventOnCompletion(_currFrame->Fence,
                                                              eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
    UpdateObjectCB(dt);
    UpdateMaterialCB(dt);
    UpdateMainPassCB(dt);

    return 0;
}

int LitColumnsScene::LateUpdate(const float& dt)
{

    return 0;
}

bool LitColumnsScene::Initialize()
{
    _pGameManager = GameManager::GetInstance();
    _renderer     = D12Renderer::GetInstance();
    _cmdList      = _renderer->GetCmdList();
    _renderer->ResetCommandList();
    _cbvSrvDescriptorSize =
        _renderer->GetDevice()->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CreateRootSignature();
    CreateShaderAndInputLayout();
    CreateShapeGeometry();
    CreateSkullGeometry();
    CreateMaterials();
    CreateRenderItmes();
    CreateFrameResources();
    CreatePSOs();
    ThrowIfFailed(_cmdList->Close());
    ID3D12CommandList* cmdlists[] = {_cmdList};
    _renderer->_commandQueue->ExecuteCommandLists(_countof(cmdlists), cmdlists);
    _renderer->FlushCommandQueue();
    ISREADYCLIENT = true;
    return true;
}

void LitColumnsScene::Render()
{
    auto cmdListAlloc = _currFrame->cmdAllocator;
    ThrowIfFailed(cmdListAlloc->Reset());
    if (isWireFrame)
    {
        ThrowIfFailed(_cmdList->Reset(cmdListAlloc.Get(),
                                      _psos["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(
            _cmdList->Reset(cmdListAlloc.Get(), _psos["opaque"].Get()));
    }
    _cmdList->RSSetViewports(1, &_renderer->_screenViewport);
    _cmdList->RSSetScissorRects(1, &_renderer->_scissorRect);

    auto dsv        = _renderer->DepthStencilView();
    auto backbufferview = _renderer->CurrentBackBufferView();
    auto backbuffer = _renderer->CurrentBackBuffer();

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backbuffer, D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    _cmdList->ResourceBarrier(1, &barrier);
    _cmdList->ClearRenderTargetView(backbufferview, Colors::LightGray, 0,
                                    nullptr);
    _cmdList->ClearDepthStencilView(
        dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0,
        nullptr);
    _cmdList->OMSetRenderTargets(1, &backbufferview, true, &dsv);
    _cmdList->SetGraphicsRootSignature(_rootSignature.Get());
    auto passCB = _currFrame->passCB->Resource();
    _cmdList->SetGraphicsRootConstantBufferView(2,
                                                passCB->GetGPUVirtualAddress());
    DrawRenderItem(_cmdList, _opaqueItems);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    _cmdList->ResourceBarrier(1, &barrier);
    ThrowIfFailed(_cmdList->Close());
    ID3D12CommandList* cmdLists[] = {_cmdList};
    _renderer->GetCommandQueue()->ExecuteCommandLists(_countof(cmdLists),
                                                      cmdLists);
    ThrowIfFailed(_renderer->_swapChain->Present(0, 0));
    _renderer->ChangeBackBuffer();
    _currFrame->Fence = ++_renderer->_fenceValue;
    _renderer->GetCommandQueue()->Signal(_renderer->_fence.Get(),
                                         _renderer->_fenceValue);

}

LitColumnsScene* LitColumnsScene::Create(HWND hWnd)
{
    LitColumnsScene* pInstance = new LitColumnsScene;
    pInstance->InitImGui(hWnd);
    return pInstance;
}

void LitColumnsScene::CreateRootSignature()
{
    CD3DX12_ROOT_PARAMETER sloatParameter[3];
    sloatParameter[0].InitAsConstantBufferView(0);
    sloatParameter[1].InitAsConstantBufferView(1);
    sloatParameter[2].InitAsConstantBufferView(2);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        3, sloatParameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ComPtr<ID3D10Blob> serializeRootSignature = nullptr;
    ComPtr<ID3D10Blob> errorblob              = nullptr;
    HRESULT hr                                = D3D12SerializeRootSignature(
        &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializeRootSignature.GetAddressOf(), errorblob.GetAddressOf());
    if (errorblob != nullptr)
    {
        ::OutputDebugStringA(
            reinterpret_cast<char*>(errorblob->GetBufferPointer()));
    }
    ThrowIfFailed(hr);
    ThrowIfFailed(_renderer->GetDevice()->CreateRootSignature(
        0, serializeRootSignature->GetBufferPointer(),
        serializeRootSignature->GetBufferSize(),
        IID_PPV_ARGS(_rootSignature.GetAddressOf())));
}

void LitColumnsScene::CreateShaderAndInputLayout()
{
    const D3D_SHADER_MACRO alphaTestDefines[] = {"ALPHA_TEST", "1", NULL, NULL};

    _shaders["standardVS"] = d3dUtil::CompileShader(
        L"../Resource/Shader/shader.hlsl", nullptr, "vs_main", "vs_5_1");
    _shaders["opaquePS"] = d3dUtil::CompileShader(
        L"../Resource/Shader/shader.hlsl", nullptr, "ps_main", "ps_5_1");
    _inputLayout = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                     D3D12_APPEND_ALIGNED_ELEMENT,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                     D3D12_APPEND_ALIGNED_ELEMENT,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
}

void LitColumnsScene::CreateShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box    = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
    GeometryGenerator::MeshData grid   = geoGen.CreateGrid(20.f, 30.f, 60, 40);
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
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
        vertices[k].pos    = box.vertices[i].position;
        vertices[k].normal = box.vertices[i].normal;
    }

    for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
    {
        vertices[k].pos    = grid.vertices[i].position;
        vertices[k].normal = grid.vertices[i].normal;
    }

    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        vertices[k].pos    = sphere.vertices[i].position;
        vertices[k].normal = sphere.vertices[i].normal;
    }

    for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
    {
        vertices[k].pos    = cylinder.vertices[i].position;
        vertices[k].normal = cylinder.vertices[i].normal;
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

void LitColumnsScene::CreateSkullGeometry()
{
    std::ifstream fin("../Resource/Models/skull.txt");
    if (!fin)
    {
        MessageBox(0, L"../Resource/Models/skull.txt 가 없습니다", 0, 0);
        return;
    }
    UINT vCount = 0;
    UINT tCount = 0;
    std::string ignore;
    fin >> ignore >> vCount;
    fin >> ignore >> tCount;
    fin >> ignore >> ignore >> ignore >> ignore;
    std::vector<Vertex> vertices(vCount);
    for (UINT i = 0; i < vCount; ++i)
    {
        fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
        fin >> vertices[i].normal.x >> vertices[i].normal.y >>
            vertices[i].normal.z;
    }
    fin >> ignore;
    fin >> ignore;
    fin >> ignore;
    std::vector<std::int32_t> indices(3 * tCount);
    for (UINT i = 0; i < tCount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    const UINT vByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
    const UINT iByteSize = static_cast<UINT>(indices.size()) * sizeof(std::int32_t);
    auto geo             = new MeshGeometry();
    geo->Name            = "skullGeo";
    ThrowIfFailed(
        D3DCreateBlob(vByteSize, geo->VertexBufferCPU.GetAddressOf()));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(),
               vByteSize);
    ThrowIfFailed(D3DCreateBlob(iByteSize, geo->IndexBufferCPU.GetAddressOf()));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(),
               iByteSize);
    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        _renderer->GetDevice(), _cmdList, vertices.data(), vByteSize,
        geo->VertexBufferUploader);
    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        _renderer->GetDevice(), _cmdList, indices.data(), iByteSize,
        geo->IndexBufferUploader);

    geo->VertexByteStride     = sizeof(Vertex);
    geo->VertexBufferByteSize = vByteSize;
    geo->IndexFormat          = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize  = iByteSize;

    SubmeshGeometry subMesh;
    subMesh.IndexCount         = static_cast<UINT>(indices.size());
    subMesh.StartIndexLocation = 0;
    subMesh.BaseVertexLocation = 0;
    geo->DrawArgs["skull"]     = subMesh;
    _geometries[geo->Name]     = std::move(geo);
}

void LitColumnsScene::CreateMaterials()
{
    auto bricks0                 = new Material();
    bricks0->Name                = "bricks0";
    bricks0->MatCBIndex          = 0;
    bricks0->DiffuseSrvHeapIndex = 0;
    bricks0->DiffuseAlbedo       = XMFLOAT4(Colors::DarkSeaGreen);
    bricks0->FresnelR0           = XMFLOAT3(0.02f, 0.02f, 0.02f);
    bricks0->Roughness           = 0.1f;

    auto stone0                 = new Material();
    stone0->Name                = "stone0";
    stone0->MatCBIndex          = 1;
    stone0->DiffuseSrvHeapIndex = 1;
    stone0->DiffuseAlbedo       = XMFLOAT4(Colors::LightSteelBlue);
    stone0->FresnelR0           = XMFLOAT3(0.05f, 0.05f, 0.05f);
    stone0->Roughness           = 0.3f;

    auto tile0                 = new Material();
    tile0->Name                = "tile0";
    tile0->MatCBIndex          = 2;
    tile0->DiffuseSrvHeapIndex = 2;
    tile0->DiffuseAlbedo       = XMFLOAT4(Colors::LightGray);
    tile0->FresnelR0           = XMFLOAT3(0.02f, 0.02f, 0.02f);
    tile0->Roughness           = 0.2f;

    auto skullMat                 = new Material();
    skullMat->Name                = "skullMat";
    skullMat->MatCBIndex          = 3;
    skullMat->DiffuseSrvHeapIndex = 3;
    skullMat->DiffuseAlbedo       = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    skullMat->FresnelR0           = XMFLOAT3(0.05f, 0.05f, 0.05);
    skullMat->Roughness           = 0.3f;

    _materials["bricks0"]  = std::move(bricks0);
    _materials["stone0"]   = std::move(stone0);
    _materials["tile0"]    = std::move(tile0);
    _materials["skullMat"] = std::move(skullMat);
}

void LitColumnsScene::CreateRenderItmes()
{
    auto boxRendeItem = new RenderItem();
    XMStoreFloat4x4(&boxRendeItem->world,
                    XMMatrixScaling(2.f, 2.f, 2.f) *
                        XMMatrixTranslation(0.f, 0.5f, 0.f));
    XMStoreFloat4x4(&boxRendeItem->texTransform,
                    XMMatrixScaling(1.f, 1.f, 1.f));
    boxRendeItem->objCbIndex    = 0;
    boxRendeItem->geo           = _geometries["shapeGeo"];
    boxRendeItem->mat           = _materials["stone0"];
    boxRendeItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    boxRendeItem->indexCount    = boxRendeItem->geo->DrawArgs["box"].IndexCount;
    boxRendeItem->startIndexLocation =
        boxRendeItem->geo->DrawArgs["box"].StartIndexLocation;
    boxRendeItem->baseVertexLocation =
        boxRendeItem->geo->DrawArgs["box"].BaseVertexLocation;
    _allRenderItem.push_back(std::move(boxRendeItem));

    auto gridRitem   = new RenderItem();
    gridRitem->world = MathHelper::Identity4x4();
    XMStoreFloat4x4(&gridRitem->texTransform, XMMatrixScaling(8.f, 8.f, 1.f));
    gridRitem->objCbIndex    = 1;
    gridRitem->mat           = _materials["tile0"];
    gridRitem->geo           = _geometries["shapeGeo"];
    gridRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->indexCount    = gridRitem->geo->DrawArgs["grid"].IndexCount;
    gridRitem->startIndexLocation =
        gridRitem->geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->baseVertexLocation =
        gridRitem->geo->DrawArgs["grid"].BaseVertexLocation;
    _allRenderItem.push_back(std::move(gridRitem));

    auto skullRenderItem = new RenderItem();
    XMStoreFloat4x4(&skullRenderItem->world,
                    XMMatrixScaling(0.5f, 0.5f, 0.5f) *
                        XMMatrixTranslation(0.f, 1.f, 0.f));
    skullRenderItem->texTransform  = Matrix::Identity;
    skullRenderItem->objCbIndex    = 2;
    skullRenderItem->mat           = _materials["skullMat"];
    skullRenderItem->geo           = _geometries["skullGeo"];
    skullRenderItem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    skullRenderItem->indexCount =
        skullRenderItem->geo->DrawArgs["skull"].IndexCount;
    skullRenderItem->startIndexLocation =
        skullRenderItem->geo->DrawArgs["skull"].StartIndexLocation;
    skullRenderItem->baseVertexLocation =
        skullRenderItem->geo->DrawArgs["skull"].BaseVertexLocation;
    _allRenderItem.push_back(std::move(skullRenderItem));
    XMMATRIX brickTexTransform = XMMatrixScaling(1.f, 1.f, 1.f);
    UINT objCBIndex            = 3;  
    for (size_t i = 0; i < 5; ++i)
    {
        auto leftCylRitem     = new RenderItem();
        auto rightCylRitem    = new RenderItem();
        auto leftSphereRitem  = new RenderItem();
        auto rightSphereRitem = new RenderItem();

        XMMATRIX leftCylWorld =
            XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
        XMMATRIX rightCylWorld =
            XMMatrixTranslation(5.f, 1.5f, -10.f + i * 5.f);

        XMMATRIX leftSphereWorld =
            XMMatrixTranslation(-5.f, 3.5f, -10.f + i * 5.f);
        XMMATRIX rightSphereWorld =
            XMMatrixTranslation(5.f, 3.5f, -10.f + i * 5.f);

        XMStoreFloat4x4(&leftCylRitem->world, leftCylWorld);
        XMStoreFloat4x4(&leftCylRitem->texTransform, brickTexTransform);
        leftCylRitem->objCbIndex    = objCBIndex++;
        leftCylRitem->geo           = _geometries["shapeGeo"];
        leftCylRitem->mat           = _materials["bricks0"];
        leftCylRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        leftCylRitem->indexCount =
            leftCylRitem->geo->DrawArgs["cylinder"].IndexCount;
        leftCylRitem->startIndexLocation =
            leftCylRitem->geo->DrawArgs["cylinder"].StartIndexLocation;
        leftCylRitem->baseVertexLocation =
            leftCylRitem->geo->DrawArgs["cylinder"].BaseVertexLocation;

        XMStoreFloat4x4(&rightCylRitem->world, rightCylWorld);
        XMStoreFloat4x4(&rightCylRitem->texTransform, brickTexTransform);
        rightCylRitem->objCbIndex    = objCBIndex++;
        rightCylRitem->geo           = _geometries["shapeGeo"];
        rightCylRitem->mat           = _materials["bricks0"];
        rightCylRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        rightCylRitem->indexCount =
            rightCylRitem->geo->DrawArgs["cylinder"].IndexCount;
        rightCylRitem->startIndexLocation =
            rightCylRitem->geo->DrawArgs["cylinder"].StartIndexLocation;
        rightCylRitem->baseVertexLocation =
            rightCylRitem->geo->DrawArgs["cylinder"].BaseVertexLocation;

        XMStoreFloat4x4(&leftSphereRitem->world, leftSphereWorld);
        XMStoreFloat4x4(&leftSphereRitem->texTransform, brickTexTransform);
        leftSphereRitem->objCbIndex    = objCBIndex++;
        leftSphereRitem->geo           = _geometries["shapeGeo"];
        leftSphereRitem->mat           = _materials["stone0"];
        leftSphereRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        leftSphereRitem->indexCount =
            leftSphereRitem->geo->DrawArgs["sphere"].IndexCount;
        leftSphereRitem->startIndexLocation =
            leftSphereRitem->geo->DrawArgs["sphere"].StartIndexLocation;
        leftSphereRitem->baseVertexLocation =
            leftSphereRitem->geo->DrawArgs["sphere"].BaseVertexLocation;

        XMStoreFloat4x4(&rightSphereRitem->world, rightSphereWorld);
        XMStoreFloat4x4(&rightSphereRitem->texTransform, brickTexTransform);
        rightSphereRitem->objCbIndex    = objCBIndex++;
        rightSphereRitem->geo           = _geometries["shapeGeo"];
        rightSphereRitem->mat           = _materials["stone0"];
        rightSphereRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        rightSphereRitem->indexCount =
            rightSphereRitem->geo->DrawArgs["sphere"].IndexCount;
        rightSphereRitem->startIndexLocation =
            rightSphereRitem->geo->DrawArgs["sphere"].StartIndexLocation;
        rightSphereRitem->baseVertexLocation =
            rightSphereRitem->geo->DrawArgs["sphere"].BaseVertexLocation;

        _allRenderItem.push_back(std::move(leftCylRitem));
        _allRenderItem.push_back(std::move(rightCylRitem));
        _allRenderItem.push_back(std::move(leftSphereRitem));
        _allRenderItem.push_back(std::move(rightSphereRitem));
    }
    for (auto& e : _allRenderItem)
    {
        _opaqueItems.push_back(e);
    }
}

void LitColumnsScene::CreateFrameResources()
{
    for (size_t i = 0; i < gNumFrameResources; ++i)
    {
        _frameResource.push_back(new FrameResource(
            _renderer->GetDevice(), 1, static_cast<UINT>(_allRenderItem.size()),
            static_cast<UINT>(_materials.size())));
    }
}

void LitColumnsScene::CreatePSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout    = {_inputLayout.data(),
                                    static_cast<UINT>(_inputLayout.size())};
    opaquePsoDesc.pRootSignature = _rootSignature.Get();
    opaquePsoDesc.VS             = {
        reinterpret_cast<BYTE*>(_shaders["standardVS"]->GetBufferPointer()),
        _shaders["standardVS"]->GetBufferSize()};
    opaquePsoDesc.PS = {
        reinterpret_cast<BYTE*>(_shaders["opaquePS"]->GetBufferPointer()),
        _shaders["opaquePS"]->GetBufferSize()};
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    opaquePsoDesc.BlendState               = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask        = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0]    = _renderer->_backBufferFormat;
    opaquePsoDesc.SampleDesc.Count = _renderer->_4xMsaaState ? 4 : 1;
    opaquePsoDesc.SampleDesc.Quality =
        _renderer->_4xMsaaState ? (_renderer->_4xMsaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat = _renderer->_depthStencilFormat;
    ThrowIfFailed(_renderer->GetDevice()->CreateGraphicsPipelineState(
        &opaquePsoDesc, IID_PPV_ARGS(_psos["opaque"].GetAddressOf())));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(_renderer->GetDevice()->CreateGraphicsPipelineState(
        &opaqueWireframePsoDesc,
        IID_PPV_ARGS(_psos["opaque_wireframe"].GetAddressOf())));
}

void LitColumnsScene::DrawRenderItem(
    ID3D12GraphicsCommandList* cmdList,
    const std::vector<RenderItem*>& renderItems)
{
    UINT objCBByte =
        d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByte =
        d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
    auto objectCB = _currFrame->objectCB->Resource();
    auto matCB    = _currFrame->materialCB->Resource();
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        auto item = renderItems[i];
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = item->geo->VertexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        D3D12_INDEX_BUFFER_VIEW indexBufferView = item->geo->IndexBufferView();
        cmdList->IASetIndexBuffer(&indexBufferView);
        cmdList->IASetPrimitiveTopology(item->primitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress =
            objectCB->GetGPUVirtualAddress() + item->objCbIndex * objCBByte;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress =
            matCB->GetGPUVirtualAddress() + item->mat->MatCBIndex * matCBByte;
        cmdList->SetGraphicsRootConstantBufferView(0,objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(1,matCBAddress);

        cmdList->DrawIndexedInstanced(item->indexCount, 1,
                                      item->startIndexLocation,
                                      item->baseVertexLocation, 0);
    }
}

void LitColumnsScene::UpdateObjectCB(const float& dt)
{
    auto currObjCB = _currFrame->objectCB;
    for (auto& iter : _allRenderItem)
    {
        if (iter->numFrameDirtyFlag > 0)
        {
            XMMATRIX world = XMLoadFloat4x4(&iter->world);
            XMMATRIX texTransform = XMLoadFloat4x4(&iter->texTransform);
            ObjectConstants cb;
            XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
            XMStoreFloat4x4(&cb.texTransform, XMMatrixTranspose(texTransform));
            currObjCB->CopyData(iter->objCbIndex, cb);
            iter->numFrameDirtyFlag--;
        }
    }
}

void LitColumnsScene::UpdateMainPassCB(const float& dt)
{
    XMMATRIX view = XMLoadFloat4x4(&_view);
    XMMATRIX proj = XMLoadFloat4x4(&_proj);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView  = XMMatrixInverse(nullptr, view);
    XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);
    XMStoreFloat4x4(&_mainPassCB.view, XMMatrixTranspose(view));
    XMStoreFloat4x4(&_mainPassCB.invView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&_mainPassCB.proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&_mainPassCB.invProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&_mainPassCB.viewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&_mainPassCB.invViewProj, XMMatrixTranspose(invViewProj));

    _mainPassCB.eyePos = _eyePos;
    _mainPassCB.renderTargetSize =
        Vector2(static_cast<float>(WINCX), static_cast<float>(WINCY));
    _mainPassCB.invRenderTargetSize = Vector2(1.f / WINCX, 1.f / WINCY);
    _mainPassCB.nearZ               = _NEAR;
    _mainPassCB.farZ                = _FAR;
    _mainPassCB.totalTime           = GameTimer::GetInstance()->TotalTime();
    _mainPassCB.delta               = dt;
    _mainPassCB.ambientLight        = {0.25f, 0.25f, 0.35f, 1.0f};
    _mainPassCB.lights[0].Direction = {0.57735f, -0.57735f, 0.57735f};
    _mainPassCB.lights[0].Strength  = {0.6f, 0.6f, 0.6f};
    _mainPassCB.lights[1].Direction = {-0.57735f, -0.57735f, 0.57735f};
    _mainPassCB.lights[1].Strength  = {0.3f, 0.3f, 0.3f};
    _mainPassCB.lights[2].Direction = {0.0f, -0.707f, -0.707f};
    _mainPassCB.lights[2].Strength  = {0.15f, 0.15f, 0.15f};

    auto currPassCB = _currFrame->passCB;
    currPassCB->CopyData(0, _mainPassCB);
}


void LitColumnsScene::UpdateMaterialCB(const float& dt) 
{
    auto currMatCB = _currFrame->materialCB;
    for (auto& iter : _materials)
    {
        Material* mat = iter.second;
        if (mat->NumFramesDirty > 0)
        {
            XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);
            MaterialConstants matConstants;
            matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
            matConstants.FresnelR0     = mat->FresnelR0;
            matConstants.Roughness     = mat->Roughness;
            XMStoreFloat4x4(&matConstants.MatTransform,
                            XMMatrixTranspose(matTransform));
            currMatCB->CopyData(mat->MatCBIndex, matConstants);
            mat->NumFramesDirty--;
        }
    }
}
