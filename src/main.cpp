//
// DirectX Playground
// Jorge Mart�nez Vargas, 22/08/2015
// Created following directxtutorial.com tutorials
//

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Screen resolution
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// global declarations
IDXGISwapChain* swapChain;             // the pointer to the swap chain interface
ID3D11Device* device;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext* deviceContext;           // the pointer to our Direct3D device context
ID3D11RenderTargetView* backBuffer;    // render target

ID3D11VertexShader* vertexShader;    // pointer to the vertex shader COM object
ID3D11PixelShader* pixelShader;     // pointer to the pixel shader COM object

ID3D11Buffer* vertexBuffer;

ID3D11InputLayout* inputLayout;

// function prototypes
void InitD3D(HWND hWindow);     // sets up and initializes Direct3D
void CleanD3D();                // closes Direct3D and releases memory
void RenderFrame();

LRESULT CALLBACK WinProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam);

#define SAFE_RELEASE(a) if((a) != NULL){ (a)->Release(); (a) = NULL; }

struct Vertex
{
    FLOAT x, y, z; // Position
    FLOAT color[4];
};

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
    // wc.hbrBackground = (HBRUSH)COLOR_WINDOW; // Disable background if running in fullscreen
    wc.lpszClassName = L"WindowClass1";

    // Register the window class
    RegisterClassEx(&wc);

    // Calculate window size for the desired client size
    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
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

    InitD3D(hWindow);

    MSG msg = {0}; // Struct to hold windows event messages

    // Game loop
    while(true)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        // Translate keystroke messages into the right format (�?)
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
            RenderFrame();
        }
    }

    CleanD3D();

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

void initShaders()
{
    // load and compile the vertex and pixel shaders
    ID3D10Blob* vs;
    ID3D10Blob* ps;
    // TODO : Handle return value (check for errors) [last param returns a blob to handle compiler errors]
    // TODO : Take a look at CreatePixelShader and pre-compiling shaders
    D3DCompileFromFile(L"VertexShader.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", 
                       "vs_4_0", 0, 0, &vs, NULL);
    D3DCompileFromFile(L"PixelShader.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", 
                       "ps_4_0", 0, 0, &ps, NULL);

    // Create shaders from blobs
    device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &vertexShader);
    device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pixelShader);

    // Set the active shaders
    deviceContext->VSSetShader(vertexShader, NULL, 0);
    deviceContext->PSSetShader(pixelShader, NULL, 0);

    // Init shader input layer (i.e. input we're going to send to vertex shader)
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, 
            D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    device->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayout);
    deviceContext->IASetInputLayout(inputLayout);
}

void InitGraphics()
{
    // create a triangle using the VERTEX struct
    Vertex vertices[] =
    {
        {0.0f, 0.5f, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f}},
        {0.45f, -0.5, 0.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
        {-0.45f, -0.5f, 0.0f, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(Vertex) * 3;             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    device->CreateBuffer(&bd, NULL, &vertexBuffer); // create the buffer

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    deviceContext->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
    memcpy(ms.pData, vertices, sizeof(vertices));                               // copy the data
    deviceContext->Unmap(vertexBuffer, NULL);                                   // unmap the buffer
}

// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWindow)
{
    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferDesc.Width = SCREEN_WIDTH;                    // set the back buffer width
    scd.BufferDesc.Height = SCREEN_HEIGHT;                  // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWindow;                             // the window to be used
    scd.SampleDesc.Count = 4;                               // how many multisamples
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL, // Flags, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SWITCH_TO_REF could be useful for debugging
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &scd,
                                  &swapChain,
                                  &device,
                                  NULL,
                                  &deviceContext);

    // get the address of the back buffer
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    device->CreateRenderTargetView(pBackBuffer, NULL, &backBuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    deviceContext->OMSetRenderTargets(1, &backBuffer, NULL);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;

    deviceContext->RSSetViewports(1, &viewport);

    initShaders();
    InitGraphics();
}

// this is the function that cleans up Direct3D and COM
void CleanD3D()
{
    // switch to windowed mode to close properly
    swapChain->SetFullscreenState(FALSE, NULL);    

    // close and release all existing COM objects
    SAFE_RELEASE(inputLayout);
    SAFE_RELEASE(vertexShader);
    SAFE_RELEASE(pixelShader);
    SAFE_RELEASE(vertexBuffer);
    SAFE_RELEASE(swapChain);
    SAFE_RELEASE(backBuffer);
    SAFE_RELEASE(device);
    SAFE_RELEASE(deviceContext);
}

// this is the function used to render a single frame
void RenderFrame(void)
{
    // Clear color
    FLOAT color[4] = {0.0f, 0.2f, 0.4f, 1.0f};
    // clear the back buffer to "color"
    deviceContext->ClearRenderTargetView(backBuffer, color);

    // do 3D rendering on the back buffer

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // select which primtive type we are using
    deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // draw the vertex buffer to the back buffer
    deviceContext->Draw(3, 0);

    // switch the back buffer and the front buffer
    swapChain->Present(0, 0);
}