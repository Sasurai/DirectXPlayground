#include "GameController.hpp"
#include "CommonDefinitions.hpp"

GameController::GameController(HINSTANCE hInstance, int cmdShow)
{
    // Window handle
    HWND hWindow;
    // Window class information struct
    WNDCLASSEX wc;

    // Clear out the window class for use (initialize all fields to null)
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    // Fill struct with the required information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw the window if it's moved
    wc.lpfnWndProc = GameController::processWinMessage;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // wc.hbrBackground = (HBRUSH)COLOR_WINDOW; // Disable background if running in fullscreen
    wc.lpszClassName = L"WindowClass1";

    // Register the window class
    RegisterClassEx(&wc);

    // Calculate window size for the desired client size
    RECT wr = {0, 0, kScreenWidth, kScreenHeight};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and save handle
    hWindow = CreateWindowEx(NULL,
                             L"WindowClass1",
                             L"DirectXPlayground", // Window title
                             WS_OVERLAPPEDWINDOW, // Window style
                             200, // x position
                             200, // y position
                             wr.right - wr.left, // width
                             wr.bottom - wr.top, // height
                             NULL, // Parant window
                             NULL, // Handle to the menu bar
                             hInstance, // App handle
                             NULL); // Something to do with multiple window creation

                                    // Display window
    ShowWindow(hWindow, cmdShow);

    _renderer.initD3D(hWindow);
}

GameController::~GameController()
{
}

LRESULT GameController::processWinMessage(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        // Close window case
    case WM_DESTROY:
        // Properly finish WinMain by sending the WM_QUIT message
        PostQuitMessage(0);
        return 0;
        break;
    }

    // Handle any message we don't handle
    return DefWindowProc(hWindow, message, wParam, lParam);
}

WPARAM GameController::gameLoop()
{
    MSG msg = {0}; // Struct to hold windows event messages

    // Game loop
    while(true)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Translate keystroke messages into the right format (¿?)
            TranslateMessage(&msg);

            // Send message to the WinProc function (registered message handler)
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
            {
                break;
            }
        }
        else
        {
            _renderer.renderFrame();
        }
    }

    return msg.wParam;
}