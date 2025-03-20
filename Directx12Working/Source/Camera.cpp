#include "Camera.h"
#include "Transform.h"
void GameCamera::FixedUpdate()
{
}
void GameCamera::Update(const float& deltaTime)
{

}
void GameCamera::LateUpdate(const float& deltaTime)
{
  _FOV = std::clamp(_FOV, 0.01f, DirectX::XM_PIDIV2);
  _projection = DirectX::XMMatrixPerspectiveFovLH(_FOV, static_cast<float>(WINCX) / static_cast<float>(WINCY), _NEAR, _FAR);
}
void GameCamera::UpdateGameCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right)
{
  Matrix rotation =
    // y rotation
    XMMatrixRotationAxis(Vector3(0.f, 1.f, 0.f), deltaX * sensitivity) *
    //x rotation
    XMMatrixRotationAxis(_right, deltaY * sensitivity);
  upDirection = XMVector3TransformNormal(upDirection, rotation);
  lookDirection = XMVector3TransformNormal(lookDirection, rotation);
  memcpy(&_view.m[0], &_right, sizeof(Vector4));
  memcpy(&_view.m[1], &upDirection, sizeof(Vector4));
  memcpy(&_view.m[2], &lookDirection, sizeof(Vector4));
  memcpy(&_view.m[3], &position, sizeof(Vector4));
  _view = DirectX::XMMatrixInverse(nullptr, _view);
}

bool GameCamera::Initialize()
{
  _pTransform = new Transform(L"Transform");
  look = { 0.f, 0.f, 1.f, 0.f };
  return true;
}

void GameCamera::Free()
{
  SafeRelease(_pTransform);
}

GameCamera* GameCamera::Create()
{
  GameCamera* pInstance = new GameCamera;
  if (pInstance->Initialize())
  {
    return pInstance;
  }

  SafeRelease(pInstance);
  return nullptr;
}
