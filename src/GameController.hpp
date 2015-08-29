#pragma once
#include <Windows.h>

#include "Renderer.hpp"

/**
    Game controller class takes care of holding the window initialization, receiving the windows
    loop events and interacting with other systems to make the game/scene work.
*/
class GameController
{
private:
    Renderer _renderer;

    // Process windows messages (events)
    static LRESULT CALLBACK processWinMessage(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam);

public:
    GameController(HINSTANCE hInstance, int cmdShow);
    ~GameController();

    WPARAM gameLoop();
};

