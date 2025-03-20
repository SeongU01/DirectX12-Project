#pragma once

//Camera

void SetPosition(DirectX::SimpleMath::Vector4 pos)
{
	GameManager::GetInstance()->SetPosition(pos);
}
void SetAtPosition(DirectX::SimpleMath::Vector4 pos)
{
	GameManager::GetInstance()->SetAtPosition(pos);
}
void UpdateCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right)
{
	GameManager::GetInstance()->UpdateCameraRotation(deltaX, deltaY, sensitivity,_right);
}
