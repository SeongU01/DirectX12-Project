#pragma once
#include "Base.h"


class Transform;
class Component;
class GameManager;

class GameObject :public Base
{
	friend class GameManager;
	friend class Component;
protected:
	explicit GameObject();
	virtual ~GameObject();
public:
	inline Transform& GetTransform() { return *_pTransform; }
	inline bool IsDead() const { return _isDead; }
	inline void SetDead() { _isDead = true; }

	template<typename T>
	T* GetComponent(std::wstring name)
	{
		for (auto& component : _components)
		{
			if (!lstrcmp(component->GetName().c_str(), name.c_str()))
				return static_cast<T*>(component);
		}

		return nullptr;
	}

	template<typename T>
	T* GetComponent()
	{
		for (auto& component : _components)
		{
			if (typeid(*component) == typeid(T))
				return static_cast<T*>(component);
		}

		return nullptr;
	}

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		T* pComponent = new T(std::forward<Args>(args)...);
		pComponent->_pOwner = this;
		pComponent->Awake();
		_components.push_back(pComponent);

		//if constexpr (std::is_base_of_v<ICollisionNotify, T>)
		//	_registeredCollisionEventComponents.push_back(pComponent);

		//if constexpr (std::is_base_of_v<Collider, T>)
		//	_colliders.push_back(pComponent);

		return pComponent;
	}

public:
	static GameObject* Create();
public://->private으로 바꾸기
	void Start();
	void FixedUpdate();
	int Update(const float& deltaTime);
	int LateUpdate(const float& deltaTime);
	void Render();
private:
	void Free()override;
private:
	std::vector<Component*>	_components;
public:
	GameManager* _pGameManager = nullptr;
protected:
	Transform* _pTransform = nullptr;
	bool _isDead = false;
	bool _isFirstInit = false;
};

#include "Transform.h"