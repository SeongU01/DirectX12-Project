#include "TestScene.h"
#include "Camera.h"
#include "Client_Define.h"
#include "D12Renderer.h"
#include "GameObject.h"
#include "Transform.h"
#include "Vertex.h"
float _NEAR = 0.01f;
float _FAR  = 1000.f;
float _FOV  = DirectX::XM_PIDIV2;

struct ObjectConstants
{
    XMFLOAT4X4 WVP = MathHelper::Identity4x4();
};

void TestScene::Free()
{
    delete _meshes;
    delete _objectCB;
    _pso->Release();
    // ImGui_ImplDX11_Shutdown();
    // ImGui_ImplWin32_Shutdown();
    // ImGui::DestroyContext();
}

bool TestScene::InitImGui(HWND hWnd)
{
    // imGui 초기화
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
    // imgui render 코드
    // ImGui::Render();
    // ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),//command List);
}

void TestScene::CreateGeometry()
{
    std::array<Vertex1, 8> vertices = {
        Vertex1({XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)}),
        Vertex1({XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black)}),
        Vertex1({XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)}),
        Vertex1({XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)}),
        Vertex1({XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)}),
        Vertex1({XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow)}),
        Vertex1({XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)}),
        Vertex1({XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta)})};

    std::array<std::uint16_t, 36> indices = {// front face
                                             0, 1, 2, 0, 2, 3,
                                             // back face
                                             4, 6, 5, 4, 7, 6,
                                             // left face
                                             4, 5, 1, 4, 1, 0,
                                             // right face
                                             3, 2, 6, 3, 6, 7,
                                             // top face
                                             1, 5, 6, 1, 6, 2,
                                             // bottom face
                                             4, 0, 3, 4, 3, 7};
    const UINT vByteSize = (UINT)vertices.size() * sizeof(Vertex1);
    const UINT iByteSize = (UINT)indices.size() * sizeof(UINT);
    _meshes              = new MeshGeometry;
    _meshes->Name        = "BoxGeometry";
    ThrowIfFailed(D3DCreateBlob(vByteSize, &_meshes->VertexBufferCPU));
    CopyMemory(_meshes->VertexBufferCPU->GetBufferPointer(), vertices.data(),
               vByteSize);
    ThrowIfFailed(D3DCreateBlob(vByteSize, &_meshes->IndexBufferCPU));
    CopyMemory(_meshes->IndexBufferCPU->GetBufferPointer(), indices.data(),
               iByteSize);
    _meshes->VertexBufferGPU = D12Renderer::GetInstance()->CreateDefaultBuffer(
        vertices.data(), vByteSize, _meshes->VertexBufferUploader);

    _meshes->IndexBufferGPU = D12Renderer::GetInstance()->CreateDefaultBuffer(
        indices.data(), iByteSize, _meshes->IndexBufferUploader);
    _meshes->VertexByteStride     = sizeof(Vertex1);
    _meshes->VertexBufferByteSize = vByteSize;
    _meshes->IndexFormat          = DXGI_FORMAT_R16_UINT;
    _meshes->IndexBufferByteSize  = iByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount         = (UINT)indices.size();
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;
    _meshes->DrawArgs["box"]   = submesh;
}

void TestScene::CreateShaderAndInputLayout()
{
    _vsCode = d3dUtil::CompileShader(L"../Resource/Shader/box.hlsl", nullptr,
                                     "vs_main", "vs_5_0");
    _psCode = d3dUtil::CompileShader(L"../Resource/Shader/box.hlsl", nullptr,
                                     "ps_main", "ps_5_0");
    _inputLayout = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
}

void TestScene::CreateConstantBuffer()
{
    _objectCB =
        new UploadBuffer<ObjectConstants>(_renderer->GetDevice(), 1, true);
    UINT objCBBytesize =
        d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress =
        _objectCB->Resource()->GetGPUVirtualAddress();
    int boxCBindex = 0;
    cbAddress += boxCBindex * objCBBytesize;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbdesc;
    cbdesc.BufferLocation = cbAddress;
    cbdesc.SizeInBytes =
        d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    _renderer->GetDevice()->CreateConstantBufferView(
        &cbdesc, _cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void TestScene::CreateDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbHeapDesc{
        .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        .NumDescriptors = 1,
        .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        .NodeMask       = 0};
    ThrowIfFailed(_renderer->GetDevice()->CreateDescriptorHeap(
        &cbHeapDesc, IID_PPV_ARGS(&_cbvHeap)));
}

void TestScene::CreateRootSignature()
{
    CD3DX12_ROOT_PARAMETER sloatRootParm[1];
    CD3DX12_DESCRIPTOR_RANGE cbTable;
    cbTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    sloatRootParm[0].InitAsDescriptorTable(1, &cbTable);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        1, sloatRootParm, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ID3D10Blob* serializeRootsig = nullptr;
    ID3D10Blob* errorblob        = nullptr;
    HRESULT hr =
        D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                    &serializeRootsig, &errorblob);
    if (errorblob != nullptr)
    {
        ::OutputDebugStringA((char*)errorblob->GetBufferPointer());
    }
    ThrowIfFailed(hr);
    ThrowIfFailed(_renderer->GetDevice()->CreateRootSignature(
        0, serializeRootsig->GetBufferPointer(),
        serializeRootsig->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));
}

void TestScene::CreatePSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc;
    ZeroMemory(&psodesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psodesc.InputLayout    = {_inputLayout.data(), (UINT)_inputLayout.size()};
    psodesc.pRootSignature = _rootSignature.Get();
    psodesc.VS = {reinterpret_cast<BYTE*>(_vsCode->GetBufferPointer()),
                  _vsCode->GetBufferSize()};
    psodesc.PS = {reinterpret_cast<BYTE*>(_psCode->GetBufferPointer()),
                  _psCode->GetBufferSize()};
    psodesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psodesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psodesc.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psodesc.SampleMask            = UINT_MAX;
    psodesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psodesc.NumRenderTargets      = 1;
    psodesc.RTVFormats[0]         = _renderer->_backBufferFormat;
    psodesc.SampleDesc.Count      = _renderer->_4xMsaaState ? 4 : 1;
    psodesc.SampleDesc.Quality =
        _renderer->_4xMsaaState ? (_renderer->_4xMsaaQuality - 1) : 0;
    psodesc.DSVFormat = _renderer->_depthStencilFormat;
    ThrowIfFailed(_renderer->GetDevice()->CreateGraphicsPipelineState(
        &psodesc, IID_PPV_ARGS(&_pso)));

}

void TestScene::Start()
{

}

int TestScene::Update(const float& dt)
{
    // client size change->modify projection matrix
    XMMATRIX p = XMMatrixPerspectiveFovLH(XM_PIDIV4, _renderer->AspectRatio(),
                                          _NEAR, _FAR);
    XMStoreFloat4x4(&_proj, p);
    if (Input::IsKeyPress(Input::MouseState::DIM_LB))
    {
        float dx = Input::GetMouseMove(Input::MouseMove::DIM_X);
        float dy = Input::GetMouseMove(Input::MouseMove::DIM_Y);

        _theta += dx * 0.005f;
        _phi += dy*  0.005f;
        _phi = MathHelper::Clamp(_phi,0.1f,XM_PI-0.1f);
    }
    else if (Input::IsKeyPress(Input::MouseState::DIM_RB))
    {
        float dx = 0.005f * Input::GetMouseMove(Input::MouseMove::DIM_X);
        float dy = 0.005f * Input::GetMouseMove(Input::MouseMove::DIM_Y);
        _radius += dx - dy;
        _radius = MathHelper::Clamp(_radius, 3.f, 15.f);
    }

    float x = _radius * sinf(_phi) * cosf(_theta);
    float y = _radius * cosf(_phi) ;
    float z = _radius * sinf(_phi) * sinf(_theta);

    XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&_view, view);

    XMMATRIX world         = XMLoadFloat4x4(&_world);
    XMMATRIX proj          = XMLoadFloat4x4(&_proj);
    XMMATRIX worldViewProj = world * view * proj;
    ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WVP, XMMatrixTranspose(worldViewProj));
    _objectCB->CopyData(0, objConstants);
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
    CreateDescriptorHeap();
    CreateConstantBuffer();
    CreateRootSignature();
    CreateGeometry();
    CreateShaderAndInputLayout();
    CreatePSO();
    ThrowIfFailed(_renderer->GetCmdList()->Close());
    ID3D12CommandList* cmdLists[] = {_renderer->GetCmdList()};
    _renderer->GetCommandQueue()->ExecuteCommandLists(_countof(cmdLists),
                                                      cmdLists);
    _renderer->FlushCommandQueue();
    
    XMMATRIX p = XMMatrixPerspectiveFovLH(XM_PIDIV4, _renderer->AspectRatio(),
                                          _NEAR, _FAR);
    XMStoreFloat4x4(&_proj, p);
    ISREADYCLIENT = true;
    return true;
}

void TestScene::Render()
{
    CD3DX12_RESOURCE_BARRIER barrier;
    ID3D12CommandAllocator* directCmdAllocator = _renderer->GetCmdalloc();
    ID3D12CommandQueue* cmdQueue               = _renderer->GetCommandQueue();
    ThrowIfFailed(directCmdAllocator->Reset());
    ThrowIfFailed(_cmdList->Reset(directCmdAllocator, _pso));
    _cmdList->RSSetViewports(1, &_renderer->_screenViewport);
    _cmdList->RSSetScissorRects(1, &_renderer->_scissorRect);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    _cmdList->ResourceBarrier(1, &barrier);
    _cmdList->ClearRenderTargetView(_renderer->CurrentBackBufferView(),
                                    DirectX::Colors::LightGray, 0, nullptr);
    _cmdList->ClearDepthStencilView(
        _renderer->DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f,0,0, nullptr);
    auto dsv = _renderer->DepthStencilView();
    auto backBuffer = _renderer->CurrentBackBufferView();
    _cmdList->OMSetRenderTargets(1, &backBuffer, true, &dsv);
    // draw box
    ID3D12DescriptorHeap* descriptorHeap[] = {_cbvHeap.Get()};
    _cmdList->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);
    _cmdList->SetGraphicsRootSignature(_rootSignature.Get());
    auto vertexBufferView = _meshes->VertexBufferView();
    auto indexBufferView  = _meshes->IndexBufferView();
    _cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
    _cmdList->IASetIndexBuffer(&indexBufferView);
    _cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _cmdList->SetGraphicsRootDescriptorTable(
        0, _cbvHeap->GetGPUDescriptorHandleForHeapStart());
    _cmdList->DrawIndexedInstanced(_meshes->DrawArgs["box"].IndexCount, 1, 0, 0,
                                   0);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _renderer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    _cmdList->ResourceBarrier(1, &barrier);
    ThrowIfFailed(_cmdList->Close());
    ID3D12CommandList* cmdLists[] = {_cmdList};
    cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
    ThrowIfFailed(_renderer->_swapChain->Present(0, 0));
    _renderer->ChangeBackBuffer();
    _renderer->FlushCommandQueue();
}

TestScene* TestScene::Create(HWND hWnd)
{
    TestScene* pInstance = new TestScene;
    pInstance->InitImGui(hWnd);
    return pInstance;
}
