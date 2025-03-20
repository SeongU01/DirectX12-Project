#include "SceneManager.h"
#include "Scene.h"
//TODO : scenamanger ���� �ۼ��ϱ�


void SceneManager::Start()
{
	//TODO : layer �ۼ��ϸ� start���ֱ� �ٲ����
	_pScene->Start();
}

void SceneManager::FixedUpdate()
{
	//TODO : layer �ۼ��ϸ� fixedupdate���ֱ�
}

int SceneManager::Update(const float& dt)
{
	if (nullptr == _pScene || !_isSetUp)
	{
		return GameState::Error;
	}

	int isEvent = 0;
	//TODO : layer �ۼ��ϸ� update���ֱ�

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
	//TODO : layer �ۼ��ϸ� LateUpdate���ֱ�
	_pScene->LateUpdate(dt);
	return 0;
}

//TODO : Render�Լ� �����
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
	//object layer ��ü���ֱ�
	_pScene = pScene;
	_pScene->Initialize();
	_isSetUp = true;
	return true;
}

void SceneManager::RemoveAll()
{
	//TODO : layer �ۼ��ϸ� RemoveAll���ֱ�
}

void SceneManager::Free()
{
	//TODO : layer�߰��ϸ� layer �����
	SafeRelease(_pScene);
}

SceneManager* SceneManager::Create()
{
	return new SceneManager;
}
