#pragma once
#include "Base.h"
using namespace DirectX::SimpleMath;
class WinApp;
class Scene;
class SceneManager;
class InputManager;
class GameTimer;
class GameCamera;
class Transform;
class D12Renderer;
class GameManager : public Base, public SingleTon<GameManager>
{
	friend class SingleTon;
public:
	struct GameDefaultSetting
	{
		HINSTANCE hInstance{};
		const TCHAR* appName{};
		int width{};
		int height{};
		int fixedCount = 50;
		bool isFullScreen = false;
		bool showCursor = true;
	};
private:
	explicit GameManager();
	virtual ~GameManager() = default;
public:
	void Run();
	bool Initialize(const GameDefaultSetting& settingInfo);
	//winapp
	HWND GetWindow() const;
	//TimeManager
	//SceneManager
	bool ChangeScene(Scene* pScene);
	void RemoveAll();

public:
	void RestoreDisplay();
private:
	void StartGame();
	void FixedUpdateGame(int count);
	int UpdateGame();
	int LateUpdateGame();
	void RenderGame();
	// Base을(를) 통해 상속됨
	void Free() override;

public:
	//camera
	void SetCameraTarget(Transform* pTransform);
	//TODO : 임시로 추가한것들 바꾸자
	void SetPosition(DirectX::SimpleMath::Vector4& pos);
	void SetAtPosition(DirectX::SimpleMath::Vector4& pos);
	void UpdateCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right);
	GameCamera* GetCurrCamera();
private:
	WinApp* _pWinApp = nullptr;
	GameCamera* _pCamera = nullptr;
	InputManager* _pInputManager = nullptr;
  GameTimer* _pTimer = nullptr;
	SceneManager* _pSceneManager = nullptr;
	D12Renderer* _pRenderer = nullptr;
	float _elapsed = 0.f;
	int _fixedCount = 0;

};


