#pragma once
#include "Base.h"
using namespace DirectX::SimpleMath;


class Transform;
class GameCamera : public Base
{
private:
	explicit GameCamera() = default;
	virtual ~GameCamera() = default;
public:
	void FixedUpdate();
	void Update(const float& deltaTime);
	void LateUpdate(const float& deltaTime);

public:
	//TODO : 임시로 넣어둔 함수이기에 나중에 타겟에 따라 회전, pos 바꿔줘라
	DirectX::SimpleMath::Vector4& GetEye() { return position; }
	void SetEye(const DirectX::SimpleMath::Vector4& pos) { position = pos; }

	DirectX::SimpleMath::Vector4& GetAt() { return targetPosition; }
	void SetAt(const DirectX::SimpleMath::Vector4& pos) { targetPosition = pos; }

	DirectX::SimpleMath::Vector4& GetUp() { return upDirection; }
	void SetUp(const DirectX::SimpleMath::Vector4& direction) { upDirection = direction; }

	DirectX::SimpleMath::Vector4& GetLook() { return lookDirection; }
	void SetLook(const DirectX::SimpleMath::Vector4& direction) { lookDirection = direction; }

	DirectX::SimpleMath::Matrix& GetView() { return _view; }
	const DirectX::SimpleMath::Matrix& GetProjection() { return _projection; }
	void SetView(const Vector4& right, const Vector4& up, const Vector4& look, const Vector4& eye)
	{
		memcpy(_view.m[0], &right, sizeof(Vector4));
		memcpy(_view.m[1], &up, sizeof(Vector4));
		memcpy(_view.m[2], &look, sizeof(Vector4));
		memcpy(_view.m[3], &eye, sizeof(Vector4));
	}

	void UpdateGameCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right);
public:
	__declspec(property(get = GetEye, put = SetEye)) DirectX::SimpleMath::Vector4 eye;
	__declspec(property(get = GetAt, put = SetAt)) DirectX::SimpleMath::Vector4 at;
	__declspec(property(get = GetUp, put = SetUp)) DirectX::SimpleMath::Vector4 up;
	__declspec(property(get = GetLook, put = SetLook)) DirectX::SimpleMath::Vector4 look;
	__declspec(property(get = GetView, put = SetView)) DirectX::SimpleMath::Matrix view;
	__declspec(property(get = GetProjection)) DirectX::SimpleMath::Matrix projection;
private:
	bool Initialize();
	// Base을(를) 통해 상속됨
	void Free() override;
public:
	static GameCamera* Create();
private:
	DirectX::SimpleMath::Vector4 position{};
	DirectX::SimpleMath::Vector4 targetPosition{};
	DirectX::SimpleMath::Vector4 upDirection{ 0.f,1.f,0.f,0.f };
	DirectX::SimpleMath::Vector4 lookDirection;
	DirectX::SimpleMath::Matrix _view;
	DirectX::SimpleMath::Matrix _projection;
	Transform* _pTarget = nullptr;
	Transform* _pTransform = nullptr;
	float yaw = 0.0f;
	float pitch = 0.0f;
};


