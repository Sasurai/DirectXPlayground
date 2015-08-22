//
// DirectX Playground
// Jorge Martínez Vargas, 22/08/2015
// Created following directxtutorial.com tutorials
//

#include <Windows.h>

LRESULT CALLBACK WinProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam);

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
    // Window handle
    HWND hWindow;
    // Window class information struct
    WNDCLASSEX wc;

    // Clear out the window class for use (initialize all fields to null)
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    // Fill struct with the required information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw the window if it's moved
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass1";

    // Register the window class
    RegisterClassEx(&wc);

    // Create the window and save handle
    hWindow = CreateWindowEx(NULL,
                             L"WindowClass1",
                             L"DirectXPlayground", // Window title
                             WS_OVERLAPPEDWINDOW, // Window style
                             300, // x position
                             300, // y position
                             600, // width
                             600, // height
                             NULL, // Parant window
                             NULL, // Handle to the menu bar
                             hInstance, // App handle
                             NULL); // Something to do with multiple window creation

    // Display window
    ShowWindow(hWindow, cmdShow);

    // Windows main loop (get messages)
    MSG msg; // Struct to hold windows event messages

    while(GetMessage(&msg, NULL, 0, 0))
    {
        // Translate keystroke messages into the right format (¿?)
        TranslateMessage(&msg);

        // Send message to the WinProc function (registered message handler)
        DispatchMessage(&msg);
    }

    // Return parameter of WM_QUIT (exit message) to Windows
    return msg.wParam;
}

/*
    WinProc, handler for windows messages (events)
*/
LRESULT CALLBACK WinProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
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