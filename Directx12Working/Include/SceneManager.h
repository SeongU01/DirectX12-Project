#pragma once
#include "Base.h"

class Scene;
class SceneManager : public Base
{
private:
	explicit SceneManager() = default;
	virtual ~SceneManager() = default;
public:
	void Start();
	void FixedUpdate();
	int Update(const float& dt);
	int LateUpdate(const float& dt);
	void Render();
	bool ChangeScene(Scene* pScene);
	void RemoveAll();
private:
	// Base을(를) 통해 상속됨
	void Free() override;
public:
	static SceneManager* Create();
private:
	Scene* _pScene = nullptr;
	bool _isSetUp = false;

};



