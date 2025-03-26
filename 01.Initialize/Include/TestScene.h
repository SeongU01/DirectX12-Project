#pragma once
#include "Scene.h"

class GameManager;
class D12Renderer;
class TestScene : public Scene
{
private:
  explicit TestScene() = default;
  virtual ~TestScene() = default;

public:
  // Scene��(��) ���� ��ӵ�

  void Start() override;
  int Update(const float& dt) override;
  int LateUpdate(const float& dt) override;
  bool Initialize() override;
  void Render() override;
  void Free() override;

private:
  bool InitImGui(HWND hWnd);
  void ImGuiRender();

public:
  static TestScene* Create(HWND hWnd);
  bool _init = false;

private:
  GameManager* _pGameManager = nullptr;
  D12Renderer* _renderer = nullptr;
};
