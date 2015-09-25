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
    , _depthStencilView(nullptr)
    , _vertexShader(nullptr)
    , _pixelShader(nullptr)
    , _indexBuffer(nullptr)
    , _vertexBuffer(nullptr)
    , _depthStencilBuffer(nullptr)
    , _inputLayout(nullptr)
    , _perObjectCBuffer(nullptr)
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
    safeRelease(_indexBuffer);
    safeRelease(_depthStencilBuffer);
    safeRelease(_swapChain);
    safeRelease(_backBuffer);
    safeRelease(_depthStencilView);
    safeRelease(_device);
    safeRelease(_deviceContext);
    safeRelease(_perObjectCBuffer);
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
    scd.SampleDesc.Count = 1;                               // how many multisamples
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

    //Describe our Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = kScreenWidth;
    depthStencilDesc.Height = kScreenHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _device->CreateTexture2D(&depthStencilDesc, NULL, &_depthStencilBuffer);
    _device->CreateDepthStencilView(_depthStencilBuffer, NULL, &_depthStencilView);

    // set the render target as the back buffer
    _deviceContext->OMSetRenderTargets(1, &_backBuffer, _depthStencilView);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = kScreenWidth;
    viewport.Height = kScreenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    _deviceContext->RSSetViewports(1, &viewport);

    //Create the buffer to send to the constant buffer to shader
    D3D11_BUFFER_DESC cbbd;
    ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

    cbbd.Usage = D3D11_USAGE_DEFAULT;
    cbbd.ByteWidth = sizeof(CBufferPerObject);
    cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbbd.CPUAccessFlags = 0;
    cbbd.MiscFlags = 0;

    _device->CreateBuffer(&cbbd, NULL, &_perObjectCBuffer);

    //Camera information
    _camPositionVec = DirectX::XMVectorSet(0.0f, 0.0f, -0.5f, 0.0f);
    _camTargetVec = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    _camUpVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    //Set the View matrix
    _camViewMatrix = DirectX::XMMatrixLookAtLH(_camPositionVec, _camTargetVec, _camUpVec);

    //Set the Projection matrix
    _camProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.4f*3.14f, 
                                                             (float)kScreenWidth / kScreenHeight, 
                                                             1.0f, 1000.0f);

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

    vs->Release();
    ps->Release();
}

void Renderer::initScene()
{
    // Create array of vertices to draw geometry
    Vertex vertices[] =
    {
        Vertex(-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
        Vertex(-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f),
        Vertex(0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
        Vertex(0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f)
    };

    // create the vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;                // write access by CPU and GPU
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;             // size is the VERTEX * vlength
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    // Create the vertices buffer
    _device->CreateBuffer(&vertexBufferDesc, NULL, &_vertexBuffer);

    // Create indexes to draw using the defined vertices
    DWORD indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    // Create the indices buffer
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;
    _device->CreateBuffer(&indexBufferDesc, &iinitData, &_indexBuffer);
    _deviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // copy the vertices into the vertex buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    _deviceContext->Map(_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
    memcpy(ms.pData, vertices, sizeof(vertices));                               // copy the data
    _deviceContext->Unmap(_vertexBuffer, NULL);                                   // unmap the buffer

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    _deviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

    // select which primtive type we are using
    _deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::renderFrame()
{
    // Clear color
    FLOAT color[4] = {0.0f, 0.2f, 0.4f, 1.0f};
    // clear the back buffer to "color"
    _deviceContext->ClearRenderTargetView(_backBuffer, color);
    // clear the depth buffer to 1 (farthest)
    _deviceContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //Set the World/View/Projection matrix, then send it to constant buffer in effect file
    // (The world matrix should be object dependent)
    _worldMatrix = DirectX::XMMatrixIdentity();
    _wvpMatrix = _worldMatrix * _camViewMatrix * _camProjectionMatrix;
    _cBufferPerObject.WVP = DirectX::XMMatrixTranspose(_wvpMatrix);
    _deviceContext->UpdateSubresource(_perObjectCBuffer, 0, NULL, &_cBufferPerObject, 0, 0);
    _deviceContext->VSSetConstantBuffers(0, 1, &_perObjectCBuffer);

    // draw the vertex buffer to the back buffer
    // Parameters are size, offset in index buffer, offset in vertex buffer
    _deviceContext->DrawIndexed(6, 0, 0); 

    // switch the back buffer and the front buffer
    _swapChain->Present(0, 0);
}