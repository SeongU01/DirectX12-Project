#pragma once
#include "Component.h"

class Transform : public Component
{
public:
	enum Property { Position, Direction, Scale, Rotation, Property_End };
public:
	explicit Transform(std::wstring name);
private:
	virtual ~Transform() = default;
	//Base을(를) 통해 상속됨
	void Free() override;
public:
	DirectX::SimpleMath::Matrix& GetWorldMatrix() { return _worldMatrix; }
	DirectX::SimpleMath::Vector4& GetPosition() { return _transform[Position]; };
	DirectX::SimpleMath::Vector4& GetCameraSpacePosition() { return _cameraSpacePosition; };
	DirectX::SimpleMath::Vector4& GetDirection() { return _transform[Direction]; }
	DirectX::SimpleMath::Vector4& GetScale() { return _transform[Scale]; }
	DirectX::SimpleMath::Vector4& GetRotation() { return _transform[Rotation]; }
	DirectX::SimpleMath::Vector4 GetWorldPosition() const;
	Transform* GetParent() { return _pParent; }

	void SetParent(Transform* parent) { _pParent = parent; }
	void SetPosition(const DirectX::SimpleMath::Vector4& position) { memcpy(&_transform[Position], &position, sizeof(DirectX::SimpleMath::Vector4)); }
	void SetCameraSpacePosition(const DirectX::SimpleMath::Vector4& position) { memcpy(&_cameraSpacePosition, &position, sizeof(DirectX::SimpleMath::Vector4)); }
	void SetDirection(const DirectX::SimpleMath::Vector4& direction) { memcpy(&_transform[Direction], &direction, sizeof(DirectX::SimpleMath::Vector4)); }
	void SetScale(const DirectX::SimpleMath::Vector4& scale) { memcpy(&_transform[Scale], &scale, sizeof(DirectX::SimpleMath::Vector4)); }
	void SetScaleSign(float x = 1.f, float y = 1.f, float z = 1.f, float w = 1.f) { _transform[Scale] *= DirectX::SimpleMath::Vector4(x, y, z, w); }
	void SetRotation(const DirectX::SimpleMath::Vector4& rotation) { memcpy(&_transform[Rotation], &rotation, sizeof(DirectX::SimpleMath::Vector4)); }

public:
	__declspec(property(get = GetPosition, put = SetPosition)) DirectX::SimpleMath::Vector4 position;
	__declspec(property(get = GetCameraSpacePosition, put = SetCameraSpacePosition)) DirectX::SimpleMath::Vector4 cameraSpacePosition;
	__declspec(property(get = GetDirection, put = SetDirection)) DirectX::SimpleMath::Vector4 direction;
	__declspec(property(get = GetRotation, put = SetRotation)) DirectX::SimpleMath::Vector4 rotation;
	__declspec(property(get = GetScale, put = SetScale)) DirectX::SimpleMath::Vector4 scale;
	__declspec(property(get = GetWorldMatrix)) DirectX::SimpleMath::Matrix  worldMatrix;
public:
	void UpdateTransform();
private:
	Transform* _pParent = nullptr;
	DirectX::SimpleMath::Vector4 _transform[Property_End];
	DirectX::SimpleMath::Vector4 _cameraSpacePosition;
	DirectX::SimpleMath::Matrix _worldMatrix{};

};


