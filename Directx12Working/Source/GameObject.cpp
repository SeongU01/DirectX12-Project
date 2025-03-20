#include "GameObject.h"
#include "GameManager.h"
#include "Component.h"

GameObject::GameObject()
	:_pGameManager(GameManager::GetInstance())
{
	_pTransform = AddComponent<Transform>(L"Transform");
}

GameObject::~GameObject()
{
	Free();
}

GameObject* GameObject::Create()
{
	return  new GameObject;
}

void GameObject::Start()
{
	if (_isFirstInit)return;
	for (auto& comps : _components)
	{
		comps->Start();
	}

}

void GameObject::FixedUpdate()
{
	if (!_isFirstInit)return;

	for (auto& comps : _components)
	{
		comps->FixedUpdate();
	}
}

int GameObject::Update(const float& deltaTime)
{
	if (!_isFirstInit)return 0;
	for (auto& comps : _components)
	{
		comps->Update(deltaTime);
	}
	return 0;
}

int GameObject::LateUpdate(const float& deltaTime)
{
	if (!_isFirstInit)return 0;

	for (auto& comps : _components)
	{
		comps->LateUpdate(deltaTime);
	}
	_pTransform->UpdateTransform();

	return 0;
}

void GameObject::Render()
{
	if (!_isFirstInit || !IsActive()) return;

	for (auto& component : _components)
	{
		if (component->IsActive())
			component->Render();
	}
}


void GameObject::Free()
{
	for (auto& comp : _components)
	{
		SafeRelease(comp);
	}
	//SafeRelease(pModel);
	_components.clear();
	_components.shrink_to_fit();
}


