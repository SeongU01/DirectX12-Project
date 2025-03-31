#pragma once
#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <wrl.h>
#include <vector>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <queue>
#include <fstream>
#include <ostream>
#include <exception>

#include <memory>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <sstream>
#include <cassert>
//directx
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
//imgui
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Function.h"



#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"
#include "functional"

extern int   WINCX;
extern int   WINCY;
extern bool  ISREADYCLIENT;
extern bool  RESIZEFLAG;
extern float _NEAR;
extern float _FAR;
extern float _FOV;

namespace GameState
{
	enum State { Normal, Error, Game_End };
}

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

typedef unsigned char _byte;
typedef unsigned short _ushort;
typedef unsigned int _uint;
typedef unsigned long _ulong;
typedef unsigned long long _ullong;

#include "D3DUtil.h"