#include "InputManager.h"

void InputManager::Update(const float& dt)
{

	if (GetForegroundWindow() != _hWnd)
	{
		// 포커스가 없을 때 입력을 무시
		_handleOut = true;
		return;
	}

	if (_handleOut)
	{
		_pMouse->GetDeviceState(sizeof(_mouseState), &_mouseState);
		_handleOut = false;
	}
	memcpy(&_prevKeyState, &_keyState, sizeof(_keyState));
	memcpy(&_prevMouseState, &_mouseState, sizeof(_mouseState));

	_pKeyBoard->GetDeviceState(sizeof(_keyState), &_keyState);
	_pMouse->GetDeviceState(sizeof(_mouseState), &_mouseState);

	DWORD result;
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	result = XInputGetState(0, &state);

	memcpy(&_prevPadState, &_padState, sizeof(GamePadState));
	_padState.button = state.Gamepad.wButtons;

	for (int i = 0; i < GamePadState::Thumb_END; i++)
	{
		short value = *(&state.Gamepad.sThumbLX + i);
		short deadZone = i > 1 ? _rightThumbDeadZone : _leftThumbDeadZone;

		if (abs(value) < deadZone)
		{
			value = 0;
		}
		_padState.thumb[i] = ClampStickValue(value);
	}

	for (int i = 0; i < GamePadState::Trigger_END; i++)
	{
		short value = *(&state.Gamepad.bLeftTrigger + i);
		if (abs(value) < _triggerThreshold)
		{
			value = 0;
		}
		_padState.trigger[i] = ClampTriggerValue(*(&state.Gamepad.bLeftTrigger + i));
	}

}

bool InputManager::SetUpInputDevice(HINSTANCE hInstance, HWND hWnd)
{
	_hWnd = hWnd;
	//keyboard device 만들고 초기화
	if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&_pInputSDK, NULL)))
		return false;
	if (FAILED(_pInputSDK->CreateDevice(GUID_SysKeyboard, &_pKeyBoard, nullptr)))
		return false;
	if (FAILED(_pKeyBoard->SetDataFormat(&c_dfDIKeyboard)))
		return false;
	if (FAILED(_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		return false;
	if (FAILED(_pKeyBoard->Acquire()))
		return false;
	//mouse device 만들고 초기화
	if (FAILED(_pInputSDK->CreateDevice(GUID_SysMouse, &_pMouse, nullptr)))
		return false;
	if (FAILED(_pMouse->SetDataFormat(&c_dfDIMouse)))
		return false;
	if (FAILED(_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		return false;
	if (FAILED(_pMouse->Acquire()))
		return false;

	return true;
}

bool InputManager::IsKeyDown(unsigned char keycord) const
{
	return!(_prevKeyState[keycord] & 0x80) && (_keyState[keycord] & 0x80);
}

bool InputManager::IsKeyDown(Input::MouseState mouseState) const
{
	return !(_prevMouseState.rgbButtons[mouseState] & 0x80) && (_mouseState.rgbButtons[mouseState] & 0x80);
}

bool InputManager::IskeyDown(Input::PadState padState) const
{
	switch (padState)
	{
	case Input::PAD_LT:
		return!(_prevPadState.trigger[GamePadState::LT] && (_padState.trigger[GamePadState::LT]));
	case Input::PAD_RT:
		return!(_prevPadState.trigger[GamePadState::RT] && (_padState.trigger[GamePadState::RT]));
	}

	return !(_prevPadState.button & padState) && (_padState.button & padState);
}

bool InputManager::IsKeyUp(unsigned char keycord) const
{
	return (_prevKeyState[keycord] & 0x80) && !(_keyState[keycord] & 0x80);
}

bool InputManager::IsKeyUp(Input::MouseState mouseState) const
{
	return (_prevMouseState.rgbButtons[mouseState] & 0x80) && !(_mouseState.rgbButtons[mouseState] & 0x80);
}

bool InputManager::IskeyUp(Input::PadState padState) const
{
	return (_prevPadState.button & padState) && !(_padState.button & padState);
}

bool InputManager::IsKeyPress(unsigned char keycord) const
{
	return (_prevKeyState[keycord] & 0x80) && (_keyState[keycord] & 0x80);
}

bool InputManager::IsKeyPress(Input::MouseState mouseState) const
{
	return (_prevMouseState.rgbButtons[mouseState] & 0x80) && (_mouseState.rgbButtons[mouseState] & 0x80);
}

bool InputManager::IskeyPress(Input::PadState padState) const
{
	return (_prevPadState.button & padState) && (_padState.button & padState);
}

float InputManager::GetAxis(Input::Axis type)
{
	float axis = 0.f;

	switch (type)
	{
	case Input::Vertical:
		axis = -_padState.thumb[GamePadState::LY];
		if (IsKeyPress(DIK_W)) axis = -1.f;
		if (IsKeyPress(DIK_S)) axis = 1.f;
		break;
	case Input::Horizontal:
		axis = _padState.thumb[GamePadState::LX];
		if (IsKeyPress(DIK_A)) axis = -1.f;
		if (IsKeyPress(DIK_D)) axis = 1.f;
		break;
	}

	return axis;
}

bool InputManager::IsMouseWheel(Input::MouseState mouseState) const
{
	switch (mouseState)
	{
	case Input::DIM_WHDN:
		if (0 > _mouseState.lZ) return true;
		break;
	case Input::DIM_WHUP:
		if (0 < _mouseState.lZ) return true;
		break;
	}

	return false;
}

float InputManager::GetMouseMove(Input::MouseMove mouseMove) const
{
	return (float)*(&_mouseState.lX + mouseMove);
}

void InputManager::SetVibration(float power)
{
	XINPUT_VIBRATION vibration{};
	USHORT vibrationPower = static_cast<USHORT>(USHORT_MAX * power);
	vibration = { vibrationPower,vibrationPower };
	XInputSetState(0, &vibration);
}

float InputManager::ClampStickValue(short& value)
{
	return std::clamp(static_cast<float>(value), -1.f, 1.f);
}

float InputManager::ClampTriggerValue(byte& value)
{
	return std::clamp(static_cast<float>(value), 0.f, 1.f);
}

void InputManager::Free()
{
	SafeRelease(_pKeyBoard);
	SafeRelease(_pMouse);
	SafeRelease(_pInputSDK);
}


