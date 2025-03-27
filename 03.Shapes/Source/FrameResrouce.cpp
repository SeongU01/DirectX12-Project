#include "FrameResrouce.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objCnt)
{
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 IID_PPV_ARGS(&cmdAllocator)));
    passCB   = new UploadBuffer<PassConstants>(device, passCount, true);
    objectCB = new UploadBuffer<ObjectConstants>(device, objCnt, true);
}

FrameResource::~FrameResource()
{
    cmdAllocator->Release();
    if (passCB)
    {
        delete passCB;
    }
    if (objectCB)
    {
        delete objectCB;
    }
}
