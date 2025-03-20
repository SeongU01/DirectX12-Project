#include "GameManager.h"
#include "WinApp.h"
#include "Camera.h"
//manager
#include "InputManager.h"
#include "GameTimer.h"
#include "SceneManager.h"
#include "D12Renderer.h"
GameManager::GameManager()
{
	_pInputManager = InputManager::GetInstance();
  _pTimer = GameTimer::GetInstance();
	_pSceneManager = SceneManager::Create();
	_pCamera = GameCamera::Create();
	_pRenderer = D12Renderer::Create();
}

void GameManager::Run()
{
	MSG msg;

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			StartGame();
			FixedUpdateGame(_fixedCount);
			if (GameState::Game_End == UpdateGame())
				break;
			if (GameState::Game_End == LateUpdateGame())
				break;
			RenderGame();
		}
	}
}

bool GameManager::Initialize(const GameDefaultSetting& settingInfo)
{
	_pWinApp = WinApp::Create(settingInfo.hInstance, settingInfo.appName, settingInfo.width, settingInfo.height, settingInfo.isFullScreen, settingInfo.showCursor);
	
	if (!_pRenderer->Initialize())
    return false;

  _pRenderer->OnResize();

	if (nullptr == _pWinApp)
	{
		throw std::runtime_error("WinApp is null!");
		return false;
	}
	if (false == _pInputManager->SetUpInputDevice(settingInfo.hInstance, _pWinApp->GetWindow()))
	{
		throw std::runtime_error("Input set fail!");
		return false;

		_fixedCount = settingInfo.fixedCount;


		return true;
	}
}
HWND GameManager::GetWindow() const
{
	return _pWinApp->GetWindow();
}

bool GameManager::ChangeScene(Scene* pScene)
{
	return _pSceneManager->ChangeScene(pScene);
}

void GameManager::RemoveAll()
{
	//layer 제거하기 removeall
}


void GameManager::RestoreDisplay()
{
	_pWinApp->RestoreDisplay();
}

void GameManager::StartGame()
{
	_pSceneManager->Start();
}

void GameManager::FixedUpdateGame(int count)
{
	_elapsed += _pTimer->DeltaTime();

	float fixed = 1.f / count;
	if (_elapsed >= fixed)
	{
		_pSceneManager->FixedUpdate();
		_pCamera->FixedUpdate();
		_elapsed -= fixed;
	}
}

int GameManager::UpdateGame()
{
	int isEvent = 0;

	_pTimer->Tick();
  float delta = _pTimer->DeltaTime();
	_pInputManager->Update(delta);
	_pCamera->Update(delta);
	isEvent = _pSceneManager->Update(delta);

	return isEvent;
}

int GameManager::LateUpdateGame()
{
	int isEvent = 0;
  float delta = _pTimer->DeltaTime();
	_pCamera->LateUpdate(delta);
	isEvent = _pSceneManager->LateUpdate(delta);
	return isEvent;
}

void GameManager::RenderGame()
{
  _pRenderer->BeginDraw();
  _pSceneManager->Render();
  _pRenderer->EndDraw();
}

void GameManager::Free()
{
	SafeRelease(_pCamera);
	SafeRelease(_pRenderer);
  SafeRelease(_pTimer);
	SafeRelease(_pInputManager);
	SafeRelease(_pSceneManager);
	SafeRelease(_pWinApp);
}

void GameManager::SetCameraTarget(Transform* pTransform)
{
}

void GameManager::SetPosition(DirectX::SimpleMath::Vector4& pos)
{
	_pCamera->SetEye(pos);
}

void GameManager::SetAtPosition(DirectX::SimpleMath::Vector4& pos)
{
	_pCamera->SetAt(pos);
}

void GameManager::UpdateCameraRotation(float deltaX, float deltaY, float sensitivity, const Vector4& _right)
{
	_pCamera->UpdateGameCameraRotation(deltaX, deltaY, sensitivity, _right);
}

GameCamera* GameManager::GetCurrCamera()
{
	return _pCamera;
}



