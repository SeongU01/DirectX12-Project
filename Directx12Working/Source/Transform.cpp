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
  // ������, ȸ��, ��ȯ ��� ���
  DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(_transform[Scale].x, _transform[Scale].y, _transform[Scale].z);

  // ���ʹϾ��� ����Ͽ� ȸ�� ��� ���
  DirectX::SimpleMath::Matrix rotation = DirectX::SimpleMath::Matrix::CreateFromQuaternion(_transform[Rotation]); // rotation�� ���ʹϾ����� ����

  DirectX::SimpleMath::Matrix translation = DirectX::SimpleMath::Matrix::CreateTranslation(_transform[Position].x, _transform[Position].y, _transform[Position].z);

  // ���� ��� ��� (������ -> ȸ�� -> ��ȯ)
  DirectX::SimpleMath::Matrix relative = scale * rotation * translation;

  // �θ� ���� ��� ���� ��Ʈ���� ������Ʈ
  if (_pParent != nullptr)
  {
    _worldMatrix = relative * _pParent->GetWorldMatrix(); // �θ��� ���� ��Ʈ���� ���
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
