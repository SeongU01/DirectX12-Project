#pragma once
#include "GameTimer.h"
#include "InputManager.h"
#include "GameManager.h"

namespace Time
{
	// TimeMgr
	inline float GetSumTime();
	inline float GetGlobalDeltaTime();
	inline void SetSumTime(float time);
	inline void SetSlowTime(float rate);
	inline void SetSlowTime(float rate, float duration);
	inline float GetDeltaTime();
	#include "Export_Time.inl"
}

namespace Input
{
	inline bool IsKeyDown(_byte keycord);
	inline bool IsKeyDown(Input::MouseState mouseState);
	inline bool IsKeyDown(Input::PadState padState);
	inline bool IsKeyUp(_byte keycord);
	inline bool IsKeyUp(Input::MouseState mouseState);
	inline bool IsKeyUp(Input::PadState padState);
	inline bool IsKeyPress(_byte keycord);
	inline bool IsKeyPress(Input::MouseState mouseState);
	inline bool IsKeyPress(Input::PadState padState);
	inline float GetAxis(Input::Axis type);
	inline float GetMouseMove(Input::MouseMove mouseMove);
	inline bool IsMouseWheel(Input::MouseState mouseState);
	inline void SetThumbDeadZone(short left, short right);
	inline void SetTriggerThreshold(byte value);
	inline void SetVibration(float power);
#include "Export_Input.inl"
}

using namespace DirectX::SimpleMath;
namespace Camera
{
	// Camera
	inline void SetAtPosition(DirectX::SimpleMath::Vector4 pos);
	inline void SetPosition(DirectX::SimpleMath::Vector4 pos);
	inline void UpdateGameCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right);
#include "Export_Camera.inl"
}