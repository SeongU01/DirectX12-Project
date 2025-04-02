#pragma once
#include "Base.h"
#include "D3DUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
struct ObjectConstants
{
    XMFLOAT4X4 world = Matrix::Identity;
    XMFLOAT4X4 texTransform = Matrix::Identity;
};

struct PassConstants
{
    XMFLOAT4X4 view = Matrix::Identity;
    XMFLOAT4X4 invView = Matrix::Identity;
    XMFLOAT4X4 proj = Matrix::Identity;
    XMFLOAT4X4 invProj = Matrix::Identity;
    XMFLOAT4X4 viewProj = Matrix::Identity;
    XMFLOAT4X4 invViewProj = Matrix::Identity;
    XMFLOAT3   eyePos = {0.f, 0.f, 0.f};
    // padding
    float    cbPerObjectPad1 = 0.f;
    XMFLOAT2 renderTargetSize = {0.f, 0.f};
    XMFLOAT2 invRenderTargetSize = {0.f, 0.f};
    float    nearZ = 0.f;
    float    farZ = 0.f;
    float    totalTime = 0.f;
    float    delta = 0.f;
    XMFLOAT4 ambientLight = {0.f, 0.f, 0.f, 1.f};
    Light    lights[MaxLights];
};

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT3 normal;
};

struct FrameResource
{
    FrameResource(ID3D12Device* device, UINT passCount, UINT objCnt,
                  UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;
    UploadBuffer<PassConstants>*                   passCB = nullptr;
    UploadBuffer<MaterialConstants>*               materialCB = nullptr;
    UploadBuffer<ObjectConstants>*                 objectCB = nullptr;
    UINT                                           Fence = 0;
};