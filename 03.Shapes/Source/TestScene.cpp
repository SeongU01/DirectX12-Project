#include "TestScene.h"
#include "Client_Define.h"
#include "D12Renderer.h"
#include "GeometryGenerator.h"
using namespace DirectX;
float _NEAR                  = 0.01f;
float _FAR                   = 1000.f;
float _FOV                   = XM_PIDIV4;
const int gNumFrameResources = 3;
void TestScene::Free()
{
    for (auto& e : _frameResources)
    {
        if (e)
        {
            delete e;
        }
    }
    for (auto& e : _geometries)
    {
        if (e.second)
        {
            delete e.second;
        }
    }
    _geometries.clear();
    for (auto& e : _allRenderItems)
    {
        if (e)
        {
            delete e;
        }
    }
    _allRenderItems.clear();
    _shaders.clear();
    _CBVHeap->Release();
    _CBVHeap = nullptr;
    _rootSignature->Release();
    _rootSignature = nullptr;
}

bool TestScene::InitImGui(HWND hWnd)
{

    return true;
}

void TestScene::ImGuiRender() {}

void TestScene::UpdateCamera(float dt)
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

void TestScene::Start() {}

int TestScene::Update(const float& dt)
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
    _currentFrameIndex = (_currentFrameIndex + 1) % gNumFrameResources;
    _currentFrame      = _frameResources[_currentFrameIndex];
    if (_currentFrame->Fence != 0 &&
        _renderer->_fence->GetCompletedValue() < _currentFrame->Fence)
    {
        HANDLE eventHandle =
            CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(_renderer->_fence->SetEventOnCompletion(
            _currentFrame->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
    UpdateObjectCBs(dt);
    UpdateMainPassCB(dt);
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

    CreateRootSignature();
    CreateInputLayoutAndShader();
    CreateShapeGeometry();
    CreateRenderItems();
    CreateFrameResource();
    CreateDescriptorHeaps();
    CreateConstantBufferViews();
    CreatePSOs();

    ThrowIfFailed(_cmdList->Close());
    ID3D12CommandList* cmdlists[] = {_cmdList};
    _renderer->_commandQueue->ExecuteCommandLists(_countof(cmdlists), cmdlists);
    _renderer->FlushCommandQueue();
    ISREADYCLIENT = true;
    return true;
}

void TestScene::Render()
{
    ID3D12GraphicsCommandList* commandList = _renderer->GetCmdList();
    auto cmdListAlloc                      = _currentFrame->cmdAllocator;
    auto dsv                               = _renderer->DepthStencilView();
    auto backBuffer                        = _renderer->CurrentBackBufferView();
    ThrowIfFailed(cmdListAlloc->Reset());
    if (isWireFrame)
    {
        ThrowIfFailed(
            commandList->Reset(cmdListAlloc, _psos["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(commandList->Reset(cmdListAlloc, _psos["opaque"].Get()));
    }
    commandList->RSSetViewports(1, &_renderer->_screenViewport);
    commandList->RSSetScissorRects(1, &_renderer->_scissorRect);
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);
    commandList->ClearRenderTargetView(backBuffer, Colors::LightGray, 0,
                                       nullptr);
    commandList->ClearDepthStencilView(
        dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0,
        nullptr);
    commandList->OMSetRenderTargets(1, &backBuffer, true, &dsv);

    ID3D12DescriptorHeap* descriptorHeaps[] = {_CBVHeap};
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootSignature(_rootSignature);
    int passCbIndex   = _passCBVOffset + _currentFrameIndex;
    auto passCbHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        _CBVHeap->GetGPUDescriptorHandleForHeapStart());
    passCbHandle.Offset(passCbIndex, _renderer->_cbvSrvUavDescriptorSize);
    commandList->SetGraphicsRootDescriptorTable(1, passCbHandle);
    DrawRenderItem(commandList, _opaqueItems);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);
    ThrowIfFailed(commandList->Close());
    ID3D12CommandList* commandLists[] = {commandList};
    _renderer->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists),
                                                      commandLists);
    ThrowIfFailed(_renderer->_swapChain->Present(0, 0));
    _renderer->ChangeBackBuffer();
    _currentFrame->Fence = ++_renderer->_fenceValue;
    _renderer->GetCommandQueue()->Signal(_renderer->_fence.Get(),
                                         _renderer->_fenceValue);
}

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
        vertices[k].pos   = box.vertices[i].position;
        vertices[k].color = XMFLOAT4(Colors::DarkOliveGreen);
    }

    for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = grid.vertices[i].position;
        vertices[k].color = XMFLOAT4(Colors::GreenYellow);
    }

    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = sphere.vertices[i].position;
        vertices[k].color = XMFLOAT4(Colors::Crimson);
    }

    for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
    {
        vertices[k].pos   = cylinder.vertices[i].position;
        vertices[k].color = XMFLOAT4(Colors::AliceBlue);
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
    auto boxRendeItem = new RenderItem();
    XMStoreFloat4x4(&boxRendeItem->world,
                    XMMatrixScaling(2.f, 2.f, 2.f) *
                        XMMatrixTranslation(0.f, 0.5f, 0.f));
    boxRendeItem->objCbIndex    = 0;
    boxRendeItem->geo           = _geometries["shapeGeo"];
    boxRendeItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    boxRendeItem->indexCount    = boxRendeItem->geo->DrawArgs["box"].IndexCount;
    boxRendeItem->startIndexLocation =
        boxRendeItem->geo->DrawArgs["box"].StartIndexLocation;
    boxRendeItem->baseVertexLocation =
        boxRendeItem->geo->DrawArgs["box"].BaseVertexLocation;
    _allRenderItems.push_back(std::move(boxRendeItem));

    auto gridRitem           = new RenderItem();
    gridRitem->world         = MathHelper::Identity4x4();
    gridRitem->objCbIndex    = 1;
    gridRitem->geo           = _geometries["shapeGeo"];
    gridRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->indexCount    = gridRitem->geo->DrawArgs["grid"].IndexCount;
    gridRitem->startIndexLocation =
        gridRitem->geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->baseVertexLocation =
        gridRitem->geo->DrawArgs["grid"].BaseVertexLocation;
    _allRenderItems.push_back(std::move(gridRitem));

    UINT objCBIndex = 2;
    for (size_t i = 0; i < 5; ++i)
    {
        auto leftCylRitem     = new RenderItem();
        auto rightCylRitem    = new RenderItem();
        auto leftSphereRitem  = new RenderItem();
        auto rightSphereRitem = new RenderItem();

        XMMATRIX leftCylWorld =
            XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f);
        XMMATRIX rightCylWorld =
            XMMatrixTranslation(5.f, 1.5f, -10.f + i * 5.f);

        XMMATRIX leftSphereWorld =
            XMMatrixTranslation(-5.f, 3.5f, -10.f + i * 5.f);
        XMMATRIX rightSphereWorld =
            XMMatrixTranslation(5.f, 3.5f, -10.f + i * 5.f);

        XMStoreFloat4x4(&leftCylRitem->world, leftCylWorld);
        leftCylRitem->objCbIndex    = objCBIndex++;
        leftCylRitem->geo           = _geometries["shapeGeo"];
        leftCylRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        leftCylRitem->indexCount =
            leftCylRitem->geo->DrawArgs["cylinder"].IndexCount;
        leftCylRitem->startIndexLocation =
            leftCylRitem->geo->DrawArgs["cylinder"].StartIndexLocation;
        leftCylRitem->baseVertexLocation =
            leftCylRitem->geo->DrawArgs["cylinder"].BaseVertexLocation;

        XMStoreFloat4x4(&rightCylRitem->world, rightCylWorld);
        rightCylRitem->objCbIndex    = objCBIndex++;
        rightCylRitem->geo           = _geometries["shapeGeo"];
        rightCylRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        rightCylRitem->indexCount =
            rightCylRitem->geo->DrawArgs["cylinder"].IndexCount;
        rightCylRitem->startIndexLocation =
            rightCylRitem->geo->DrawArgs["cylinder"].StartIndexLocation;
        rightCylRitem->baseVertexLocation =
            rightCylRitem->geo->DrawArgs["cylinder"].BaseVertexLocation;

        XMStoreFloat4x4(&leftSphereRitem->world, leftSphereWorld);
        leftSphereRitem->objCbIndex    = objCBIndex++;
        leftSphereRitem->geo           = _geometries["shapeGeo"];
        leftSphereRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        leftSphereRitem->indexCount =
            leftSphereRitem->geo->DrawArgs["sphere"].IndexCount;
        leftSphereRitem->startIndexLocation =
            leftSphereRitem->geo->DrawArgs["sphere"].StartIndexLocation;
        leftSphereRitem->baseVertexLocation =
            leftSphereRitem->geo->DrawArgs["sphere"].BaseVertexLocation;

        XMStoreFloat4x4(&rightSphereRitem->world, rightSphereWorld);
        rightSphereRitem->objCbIndex    = objCBIndex++;
        rightSphereRitem->geo           = _geometries["shapeGeo"];
        rightSphereRitem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        rightSphereRitem->indexCount =
            rightSphereRitem->geo->DrawArgs["sphere"].IndexCount;
        rightSphereRitem->startIndexLocation =
            rightSphereRitem->geo->DrawArgs["sphere"].StartIndexLocation;
        rightSphereRitem->baseVertexLocation =
            rightSphereRitem->geo->DrawArgs["sphere"].BaseVertexLocation;

        _allRenderItems.push_back(std::move(leftCylRitem));
        _allRenderItems.push_back(std::move(rightCylRitem));
        _allRenderItems.push_back(std::move(leftSphereRitem));
        _allRenderItems.push_back(std::move(rightSphereRitem));
    }
    for (auto& e : _allRenderItems)
    {
        _opaqueItems.push_back(e);
    }
}

void TestScene::CreateDescriptorHeaps()
{
    UINT objCount = static_cast<UINT>(_opaqueItems.size());
    // 각 프레임 자원의 물체마다 상수버퍼 뷰의 서술자가 필요.
    // +1은 각 프레임 자원에 필요한 패스별 상수 버퍼 뷰를 위한것.
    UINT numDescriptors = (objCount + 1) * gNumFrameResources;

    _passCBVOffset = objCount * gNumFrameResources;
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.NumDescriptors = numDescriptors;
    desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask       = 0;
    ThrowIfFailed(_renderer->GetDevice()->CreateDescriptorHeap(
        &desc, IID_PPV_ARGS(&_CBVHeap)));
}

void TestScene::CreateConstantBufferViews()
{
    UINT byteSize =
        d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT objCount = static_cast<UINT>(_opaqueItems.size());
    for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
    {
        // resource의 업로드 버퍼.
        ID3D12Resource* objectCB =
            _frameResources[frameIndex]->objectCB->Resource();
        for (UINT i = 0; i < objCount; ++i)
        {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress =
                objectCB->GetGPUVirtualAddress();
            // 현재 버퍼에서 i번째 오브젝트별 상수 버퍼의 오프셋을 가상주소에
            // 더함
            cbAddress += i * byteSize;
            // 서술자 힙에서 i번째 물체별 CBV의 오프셋
            int heapIdx = frameIndex * objCount + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                _CBVHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIdx, _renderer->_cbvSrvUavDescriptorSize);
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc{.BufferLocation = cbAddress,
                                                 .SizeInBytes    = byteSize};
            _renderer->GetDevice()->CreateConstantBufferView(&desc, handle);
        }
    }

    byteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
    for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
    {
        ID3D12Resource* passCB =
            _frameResources[frameIndex]->passCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();
        int heapIdx                         = _passCBVOffset + frameIndex;
        auto handle                         = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            _CBVHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(heapIdx, _renderer->_cbvSrvUavDescriptorSize);
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc{.BufferLocation = cbAddress,
                                             .SizeInBytes    = byteSize};
        _renderer->GetDevice()->CreateConstantBufferView(&desc, handle);
    }
}

void TestScene::CreateFrameResource()
{
    for (size_t i = 0; i < gNumFrameResources; ++i)
    {
        _frameResources.push_back(
            new FrameResource(_renderer->GetDevice(), 1,
                              static_cast<UINT>(_allRenderItems.size())));
    }
}

void TestScene::CreateRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE cbTable0;
    cbTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE cbTable1;
    cbTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[2];
    slotRootParameter[0].InitAsDescriptorTable(1, &cbTable0);
    slotRootParameter[1].InitAsDescriptorTable(1, &cbTable1);

    CD3DX12_ROOT_SIGNATURE_DESC desc(
        2, slotRootParameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ID3D10Blob* serializeRoosig = nullptr;
    ID3D10Blob* errorBlob       = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &desc, D3D_ROOT_SIGNATURE_VERSION_1, &serializeRoosig, &errorBlob);
    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);
    ThrowIfFailed(_renderer->GetDevice()->CreateRootSignature(
        0, serializeRoosig->GetBufferPointer(),
        serializeRoosig->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));
}

void TestScene::CreatePSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout    = {_inputLayout.data(),
                                    static_cast<UINT>(_inputLayout.size())};
    opaquePsoDesc.pRootSignature = _rootSignature;
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

void TestScene::CreateInputLayoutAndShader()
{
    _shaders["standardVS"] = d3dUtil::CompileShader(
        L"../Resource/Shader/shader.hlsl", nullptr, "vs_main", "vs_5_1");
    _shaders["opaquePS"] = d3dUtil::CompileShader(
        L"../Resource/Shader/shader.hlsl", nullptr, "ps_main", "ps_5_1");
    _inputLayout = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
                     D3D12_APPEND_ALIGNED_ELEMENT,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
}

void TestScene::DrawRenderItem(ID3D12GraphicsCommandList* cmdList,
                               const std::vector<RenderItem*>& items)
{
    UINT bytesize =
        d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = _currentFrame->objectCB->Resource();
    for (size_t i = 0; i < items.size(); ++i)
    {
        auto item             = items[i];
        auto vertexBufferView = item->geo->VertexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        auto indexBufferView = item->geo->IndexBufferView();
        cmdList->IASetIndexBuffer(&indexBufferView);
        cmdList->IASetPrimitiveTopology(item->primitiveType);
        UINT cbIndex =
            _currentFrameIndex * static_cast<UINT>(_opaqueItems.size()) +
            item->objCbIndex;
        auto cbHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            _CBVHeap->GetGPUDescriptorHandleForHeapStart());
        cbHandle.Offset(cbIndex, _renderer->_cbvSrvUavDescriptorSize);
        cmdList->SetGraphicsRootDescriptorTable(0, cbHandle);
        cmdList->DrawIndexedInstanced(item->indexCount, 1,
                                      item->startIndexLocation,
                                      item->baseVertexLocation, 0);
    }
}

void TestScene::UpdateMainPassCB(const float& dt)
{
    XMMATRIX view = XMLoadFloat4x4(&_view);
    XMMATRIX proj = XMLoadFloat4x4(&_proj);

    XMMATRIX viewProj    = XMMatrixMultiply(view, proj);
    XMMATRIX invView     = XMMatrixInverse(nullptr, view);
    XMMATRIX invProj     = XMMatrixInverse(nullptr, proj);
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);
    XMStoreFloat4x4(&_mainPassCB.view, XMMatrixTranspose(view));
    XMStoreFloat4x4(&_mainPassCB.proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&_mainPassCB.invView, XMMatrixTranspose(invView));
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
    auto currPassCB                 = _currentFrame->passCB;
    currPassCB->CopyData(0, _mainPassCB);
}

void TestScene::UpdateObjectCBs(const float& dt)
{
    auto currObjCB = _currentFrame->objectCB;
    for (auto& e : _allRenderItems)
    {
        if (e->numFrameDirtyFlag > 0)
        {
            XMMATRIX world = XMLoadFloat4x4(&e->world);
            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));
            currObjCB->CopyData(e->objCbIndex, objConstants);
            e->numFrameDirtyFlag--;
        }
    }
}
