//
// DirectX Playground
// Jorge Martínez Vargas, 22/08/2015
// Created following directxtutorial.com tutorials
//

#include <Windows.h>

#include "GameController.hpp"

/*
    WinMain (handle app startup)

    hInstance : Handle to the app instance
    hPrevInstance : NULL (legacy parameter)
    cmdLine : Command line call as string
    cmdShow : How the window should appear, see documentation for possible values
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR cmdLine, int cmdShow)
{
    GameController gameController(hInstance, cmdShow);

    // Return parameter of WM_QUIT (exit message) to Windows
    return gameController.gameLoop();
}
