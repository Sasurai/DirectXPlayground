#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Renderer.hpp"
#include "CommonDefinitions.hpp"

Renderer::Renderer()
    : _swapChain(nullptr)
    , _device(nullptr)
    , _deviceContext(nullptr)
    , _backBuffer(nullptr)
    , _vertexShader(nullptr)
    , _pixelShader(nullptr)
    , _vertexBuffer(nullptr)
    , _inputLayout(nullptr)
{
}

Renderer::~Renderer()
{
    // switch to windowed mode to close properly
    if(_swapChain)
    {
        _swapChain->SetFullscreenState(FALSE, NULL);
    }

    // close and release all existing COM objects
    safeRelease(_inputLayout);
    safeRelease(_vertexShader);
    safeRelease(_pixelShader);
    safeRelease(_vertexBuffer);
    safeRelease(_swapChain);
    safeRelease(_backBuffer);
    safeRelease(_device);
    safeRelease(_deviceContext);
}

void Renderer::initD3D(HWND hWindow)
{
    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferDesc.Width = kScreenWidth;                    // set the back buffer width
    scd.BufferDesc.Height = kScreenHeight;                  // set the back buffer height
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
                                  &_swapChain,
                                  &_device,
                                  NULL,
                                  &_deviceContext);

    // get the address of the back buffer
    ID3D11Texture2D *pBackBuffer;
    _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    _device->CreateRenderTargetView(pBackBuffer, NULL, &_backBuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    _deviceContext->OMSetRenderTargets(1, &_backBuffer, NULL);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = kScreenWidth;
    viewport.Height = kScreenHeight;

    _deviceContext->RSSetViewports(1, &viewport);

    initShaders();
    initScene();
}

void Renderer::initShaders()
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
    _device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &_vertexShader);
    _device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &_pixelShader);

    // Set the active shaders
    _deviceContext->VSSetShader(_vertexShader, NULL, 0);
    _deviceContext->PSSetShader(_pixelShader, NULL, 0);

    // Init shader input layer (i.e. input we're going to send to vertex shader)
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
        D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    _device->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &_inputLayout);
    _deviceContext->IASetInputLayout(_inputLayout);
}

void Renderer::initScene()
{
    // create a triangle using the VERTEX struct
    Vertex vertices[] =
    {
        {0.0f, 0.5f, 0.0f,{1.0f, 0.0f, 0.0f, 1.0f}},
        {0.45f, -0.5, 0.0f,{0.0f, 1.0f, 0.0f, 1.0f}},
        {-0.45f, -0.5f, 0.0f,{0.0f, 0.0f, 1.0f, 1.0f}}
    };

    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access by CPU and GPU
    bd.ByteWidth = sizeof(Vertex) * 3;             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    _device->CreateBuffer(&bd, NULL, &_vertexBuffer); // create the buffer

                                                    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    _deviceContext->Map(_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
    memcpy(ms.pData, vertices, sizeof(vertices));                               // copy the data
    _deviceContext->Unmap(_vertexBuffer, NULL);                                   // unmap the buffer
}

void Renderer::renderFrame()
{
    // Clear color
    FLOAT color[4] = {0.0f, 0.2f, 0.4f, 1.0f};
    // clear the back buffer to "color"
    _deviceContext->ClearRenderTargetView(_backBuffer, color);

    // do 3D rendering on the back buffer

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    _deviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

    // select which primtive type we are using
    _deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // draw the vertex buffer to the back buffer
    _deviceContext->Draw(3, 0);

    // switch the back buffer and the front buffer
    _swapChain->Present(0, 0);
}