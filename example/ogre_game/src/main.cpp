#include<Ogre.h>
#include"test_game.h"

TestGame* game;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmd, int show)
{
#if defined(WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TestLogicSystem ls;
	game = new TestGame();
	game->Initialize("test_game", &ls);
	game->Run();
	game->UnInitialize();
	delete game;
	return 0;
}