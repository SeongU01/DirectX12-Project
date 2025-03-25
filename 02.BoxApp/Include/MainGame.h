#pragma once
#include "Base.h"


class GameManager;

class MainGame : public Base
{
protected:
	explicit MainGame();
	virtual ~MainGame() = default;
public:
	void Run();

private:
	bool Initailize(HINSTANCE hInstance);
	// Base��(��) ���� ��ӵ�
	void Free() override;
public:
	static MainGame* Create(HINSTANCE hInstance);
private:
	GameManager* _pGameManager = nullptr;
};

