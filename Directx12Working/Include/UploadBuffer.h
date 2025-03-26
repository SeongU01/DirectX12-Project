#pragma once
#include "d3dUtil.h"

template <typename T>
class UploadBuffer
{
public:
  UploadBuffer(ID3D12Device* device, UINT elemCount, bool isConstantBuffer)
      : _isConstantBuffer{isConstantBuffer}
  {
    _elemByteSize = sizeof(T);
    // 상수 버퍼 원소의 크기는 반드시 256바이트의 배수여야한다.
    // 하드웨어가 m*256바이트 오프셋에서 시작하는 상수 자료만 볼 수 있기 때문.
    // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
    // UINT64 OffsetInBytes; // 256의 배수
    // UINT   SizeInBytes;   // 256의 배수
    // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
    if (isConstantBuffer)
    {
      _elemByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));
    }
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer(_elemByteSize * elemCount);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(_uploadBuffer.GetAddressOf())));

    ThrowIfFailed(
        _uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_mappedData)));
  }
  UploadBuffer(const UploadBuffer& rhs) = delete;
  UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
  ~UploadBuffer()
  {
    if (nullptr != _uploadBuffer)
    {
      _uploadBuffer->Unmap(0, nullptr);
    }
    _mappedData = nullptr;
  }
  ID3D12Resource* Resource() const { return _uploadBuffer.Get(); }

  void CopyData(int elementIndex, const T& data)
  {
    memcpy(&_mappedData[elementIndex * _elemByteSize], &data, sizeof(T));
  }

private:
  Microsoft::WRL::ComPtr<ID3D12Resource> _uploadBuffer;
  BYTE* _mappedData = nullptr;
  UINT _elemByteSize = 0;
  bool _isConstantBuffer = false;
};