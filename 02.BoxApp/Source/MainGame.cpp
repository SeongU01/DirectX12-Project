#include "MainGame.h"
#include "TestScene.h"

//Manager
#include "GameManager.h"


MainGame::MainGame()
	: _pGameManager(GameManager::GetInstance())
{
}
void MainGame::Run()
{
	_pGameManager->Run();
}

bool MainGame::Initailize(HINSTANCE hInstance)
{
	GameManager::GameDefaultSetting info;
	info.hInstance = hInstance;
	info.appName = L"Test";
	info.width = WINCX;
	info.height = WINCY;
	info.fixedCount = 50;
	info.isFullScreen = false;

	_pGameManager->Initialize(info);
	_pGameManager->ChangeScene(TestScene::Create(_pGameManager->GetWindow()));
	return true;
}

void MainGame::Free()
{
	SafeRelease(_pGameManager);
}

MainGame* MainGame::Create(HINSTANCE hInstance)
{
	MainGame* pInstance = new MainGame;
	if (pInstance->Initailize(hInstance))
		return pInstance;

	SafeRelease(pInstance);
	return nullptr;
}
