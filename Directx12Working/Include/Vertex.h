#pragma once
#include "Base.h"
using namespace DirectX::SimpleMath;
struct Vertex1
{
  Vector3 pos;
  Vector4 color;
};

struct Vertex2
{
  Vector3 pos;
  Vector3 normal;
  Vector2 tex1;
  Vector2 tex2;
};