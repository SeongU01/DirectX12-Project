#pragma once

bool IsKeyDown(_byte keycord)
{
	return InputManager::GetInstance()->IsKeyDown(keycord);
}
bool IsKeyDown(Input::MouseState mouseState)
{
	return InputManager::GetInstance()->IsKeyDown(mouseState);
}
bool IsKeyDown(Input::PadState padState)
{
	return InputManager::GetInstance()->IsKeyDown(padState);
}
bool IsKeyUp(_byte keycord)
{
	return InputManager::GetInstance()->IsKeyUp(keycord);
}
bool IsKeyUp(Input::MouseState mouseState)
{
	return InputManager::GetInstance()->IsKeyUp(mouseState);
}
bool IsKeyUp(Input::PadState padState)
{
	return InputManager::GetInstance()->IsKeyUp(padState);
}
bool IsKeyPress(_byte keycord)
{
	return InputManager::GetInstance()->IsKeyPress(keycord);
}
bool IsKeyPress(Input::MouseState mouseState)
{
	return InputManager::GetInstance()->IsKeyPress(mouseState);
}
bool IsKeyPress(Input::PadState padState)
{
	return InputManager::GetInstance()->IsKeyPress(padState);
}
float GetAxis(Input::Axis type)
{
	return InputManager::GetInstance()->GetAxis(type);
}
float GetMouseMove(Input::MouseMove mouseMove)
{
	return InputManager::GetInstance()->GetMouseMove(mouseMove);
}
bool IsMouseWheel(Input::MouseState mouseState)
{
	return InputManager::GetInstance()->IsMouseWheel(mouseState);
}
void SetThumbDeadZone(short left, short right)
{
	InputManager::GetInstance()->SetThumbDeadZone(left, right);
}
void SetTriggerThreshold(byte value)
{
	InputManager::GetInstance()->SetTriggerThreshold(value);
}
void SetVibration(float power)
{
	InputManager::GetInstance()->SetVibration(power);
}