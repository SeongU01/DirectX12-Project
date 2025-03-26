#include "MainGame.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

void CheckMemoryLeaks()
{
	_CrtCheckMemory();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//_CrtSetBreakAlloc(10645);
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);



	MainGame* pMainGame = MainGame::Create(hInstance);
	pMainGame->Run();

	SafeRelease(pMainGame);
	//atexit(CheckMemoryLeaks);

	return EXIT_SUCCESS;
}