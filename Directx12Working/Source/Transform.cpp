#include "Transform.h"
#include "GameManager.h"
#include "Camera.h"
Transform::Transform(std::wstring name)
  :Component(name)
{
  _transform[Scale] = { 1.f,1.f,1.f,1.f };
}

void Transform::Free()
{
}

DirectX::SimpleMath::Vector4 Transform::GetWorldPosition() const
{
  DirectX::SimpleMath::Vector4 position{};
  memcpy(&position, _worldMatrix.m[3], sizeof(DirectX::SimpleMath::Vector4));

  return position;
}

void Transform::UpdateTransform()
{
  // 스케일, 회전, 변환 행렬 계산
  DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(_transform[Scale].x, _transform[Scale].y, _transform[Scale].z);

  // 쿼터니언을 사용하여 회전 행렬 계산
  DirectX::SimpleMath::Matrix rotation = DirectX::SimpleMath::Matrix::CreateFromQuaternion(_transform[Rotation]); // rotation을 쿼터니언으로 변경

  DirectX::SimpleMath::Matrix translation = DirectX::SimpleMath::Matrix::CreateTranslation(_transform[Position].x, _transform[Position].y, _transform[Position].z);

  // 월드 행렬 계산 (스케일 -> 회전 -> 변환)
  DirectX::SimpleMath::Matrix relative = scale * rotation * translation;

  // 부모가 있을 경우 월드 매트릭스 업데이트
  if (_pParent != nullptr)
  {
    _worldMatrix = relative * _pParent->GetWorldMatrix(); // 부모의 월드 매트릭스 사용
  }
  else
  {
    _worldMatrix = relative;
  }
  DirectX::SimpleMath::Matrix viewMatrix = GameManager::GetInstance()->GetCurrCamera()->view;
  DirectX::SimpleMath::Vector4 worldPosition = DirectX::SimpleMath::Vector4(_worldMatrix._41, _worldMatrix._42, _worldMatrix._43, 1.f);
  DirectX::SimpleMath::Vector4 cameraSpacePosition = DirectX::SimpleMath::Vector4::Transform(worldPosition, viewMatrix);
  _cameraSpacePosition = cameraSpacePosition;
}
