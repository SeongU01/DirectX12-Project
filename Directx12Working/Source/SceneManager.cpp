#include "SceneManager.h"
#include "Scene.h"
//TODO : scenamanger 마저 작성하기


void SceneManager::Start()
{
	//TODO : layer 작성하면 start해주기 바꿔야함
	_pScene->Start();
}

void SceneManager::FixedUpdate()
{
	//TODO : layer 작성하면 fixedupdate해주기
}

int SceneManager::Update(const float& dt)
{
	if (nullptr == _pScene || !_isSetUp)
	{
		return GameState::Error;
	}

	int isEvent = 0;
	//TODO : layer 작성하면 update해주기

	_pScene->Update(dt);
	return 0;
}

int SceneManager::LateUpdate(const float& dt)
{
	if (nullptr == _pScene || !_isSetUp)
	{
		return GameState::Error;
	}

	int isEvent = 0;
	//TODO : layer 작성하면 LateUpdate해주기
	_pScene->LateUpdate(dt);
	return 0;
}

//TODO : Render함수 지우기
void SceneManager::Render()
{
	_pScene->Render();
}

bool SceneManager::ChangeScene(Scene* pScene)
{
	if (nullptr == pScene)
	{
		return false;
	}
	if (nullptr != _pScene)
	{
		SafeRelease(pScene);
	}
	//object layer 교체해주기
	_pScene = pScene;
	_pScene->Initialize();
	_isSetUp = true;
	return true;
}

void SceneManager::RemoveAll()
{
	//TODO : layer 작성하면 RemoveAll해주기
}

void SceneManager::Free()
{
	//TODO : layer추가하면 layer 지우기
	SafeRelease(_pScene);
}

SceneManager* SceneManager::Create()
{
	return new SceneManager;
}
